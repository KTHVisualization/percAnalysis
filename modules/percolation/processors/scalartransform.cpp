/*********************************************************************
 *  Author  : Anke Friederici & Tino Weinkauf
 *  Init    : Tuesday, February 20, 2018 - 23:41:57
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <percolation/processors/scalartransform.h>
#include <percolation/processors/rawpercolationloader.h>
#include <modules/discretedata/dataset.h>
#include <modules/discretedata/connectivity/structuredgrid.h>
#include <modules/discretedata/connectivity/elementiterator.h>
#include <modules/kxtools/performancetimer.h>
#include <inviwo/core/util/filesystem.h>

#ifndef __clang__
#include <omp.h>
#endif

namespace inviwo {
using namespace discretedata;

const std::unordered_map<std::string, ScalarTransform::ScalarFunc> ScalarTransform::ScalarVariants(
    {{"uv", [](const std::array<double, 3>& data,
               const std::array<double, 3>&) { return data[0] * data[1]; }},
     {"uw", [](const std::array<double, 3>& data,
               const std::array<double, 3>&) { return data[0] * data[2]; }},
     {"vw", [](const std::array<double, 3>& data,
               const std::array<double, 3>&) { return data[1] * data[2]; }},

     {"v2w2", [](const std::array<double, 3>& data,
                 const std::array<double, 3>&) { return data[0] * data[0] - data[1] * data[1]; }},
     {"K",
      [](const std::array<double, 3>&, const std::array<double, 3>& rawData) {
          return 0.5 * (rawData[0] * rawData[0] + rawData[1] * rawData[1]);
      }},  // No average!!! But divide by the "rms" file
     {"k", [](const std::array<double, 3>& data, const std::array<double, 3>&) {
          return 0.5 * (data[0] * data[0] + data[1] * data[1] + data[2] * data[2]);
      }}});

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo ScalarTransform::processorInfo_{
    "org.inviwo.ScalarTransform",  // Class identifier
    "Scalar Transform",            // Display name
    "Percolation",                 // Category
    CodeState::Experimental,       // Code state
    Tags::None,                    // Tags
};

const ProcessorInfo ScalarTransform::getProcessorInfo() const { return processorInfo_; }

ScalarTransform::ScalarTransform()
    : Processor()
    , portInData("InData")
    , portOutData("OutData")
    , rmsName("fileName", "RMS File")
    , negInfForUndefined("negInfForUndefined", "-Inf for Undefined RMS", true)
    , undefinedValue("undefinedValue", "Undefined Value", -1, -10, 10) {
    addPort(portInData);
    addPort(portOutData);
    addProperty(rmsName);
    addProperty(negInfForUndefined);
    addProperty(undefinedValue);
    undefinedValue.visibilityDependsOn(negInfForUndefined, [](auto& p) { return !p.get(); });
    rmsName.setAcceptMode(AcceptMode::Open);

    rmsName.onChange([this]() { this->OnChangeRMS(); });
}

void ScalarTransform::process() {
    // Get data
    auto inData = portInData.getData();
    if (!inData || inData->getNumChannels() < 2) return;

    // New output dataset.
    auto outData = std::make_shared<DataSet>(*inData.get());
    auto velocity = inData->getAsBuffer<double, 3>("Velocity", GridPrimitive::Vertex);
    auto avgVelocity = inData->getAsBuffer<double, 3>("AveragedVelocity", GridPrimitive::Vertex);

    // Could not match file name to known scalar variant.
    if (!_currentScalar.second) {
        LogWarn("Invalid rms file. Aborting.");
        return;
    }

    // Transform velocity into selected scalar.
    BufferChannel<double, 1>* percolationScalar =
        new BufferChannel<double, 1>(velocity->size(), "PercolationScalar", GridPrimitive::Vertex);
    auto regGrid = dynamic_cast<const StructuredGrid<3>*>(inData->getGrid().get());
    if (!regGrid) {
        LogWarn("Not a regular grid as expected. Aborting.");
        return;
    }
    ind numDimensions = (ind)regGrid->getDimension();

    if (numDimensions != 3) {
        LogWarn("Not a 3D grid. Aborting.");
        return;
    }

    // Load rms.
    ivec3 numVertsVec(regGrid->getNumVerticesInDimension(0), regGrid->getNumVerticesInDimension(1),
                      1);
    ind numVertsXY = (numVertsVec.x * numVertsVec.y);
    double* rmsBuffer =
        RawPercolationLoader::loadComponent(rmsName.get(), numVertsVec, 0, nullptr, true);

    if (!rmsBuffer) {
        LogWarn("Could not load given file. Aborting.");
        return;
    }

    // Data okay. Start iterating.
    for (ind z = 0; z <= regGrid->getNumVerticesInDimension(2) - 1; ++z)
        for (ind xy = 0; xy < numVertsXY; ++xy) {
            ind xyz = xy + z * numVertsXY;

            // Load RMS value/mask.
            double rms = rmsBuffer[xy];
            IVW_ASSERT(rms >= 0, "RMS not positive: " << rms);

            // Check if valid.
            if (rms == 0) {
                percolationScalar->get(xyz) = negInfForUndefined.get()
                                                  ? -std::numeric_limits<double>::max()
                                                  : undefinedValue.get();
                continue;
            }

            // Divide by RMS.
            const std::array<double, 3>& velocityXYZ = velocity->get<std::array<double, 3>>(xyz);
            const std::array<double, 3>& AvgVelocityXYZ =
                avgVelocity->get<std::array<double, 3>>(xyz);
            double scalar = abs(_currentScalar.second(AvgVelocityXYZ, velocityXYZ));
            percolationScalar->get(xyz) = scalar / rms;
        }

    // Finished, add to output.
    outData->addChannel(percolationScalar);
    portOutData.setData(outData);
}

void ScalarTransform::OnChangeRMS() {
    // Get type of scalar. Sizeing is important!
    std::string fileName = inviwo::filesystem::getFileNameWithExtension(rmsName.get());
    size_t compLength = fileName.find('_', 0);
    std::string component = fileName.substr(0, compLength);

    // Find the corresponding function.
    bool found = false;

    for (auto& variant : ScalarVariants)
        if (!component.compare(variant.first)) {
            _currentScalar = variant;
            found = true;
            break;
        }

    // Could not find a match.
    if (!found) {
        LogWarn("Unknown scalar component name \")" << component << '\"');
        _currentScalar = std::make_pair("Invalid", nullptr);
        return;
    }
}

}  // namespace inviwo
