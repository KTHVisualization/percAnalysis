/*********************************************************************
 *  Author  : Anke Friederici & Tino Weinkauf
 *  Init    : Tuesday, February 20, 2018 - 23:41:57
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <percolation/processors/rawpercolationloader.h>
#include <modules/discretedata/dataset.h>
#include <modules/discretedata/connectivity/periodicgrid.h>
#include <modules/discretedata/connectivity/elementiterator.h>
#include <modules/kxtools/performancetimer.h>
#include <inviwo/core/util/filesystem.h>
#include <modules/discretedata/connectivity/euclideanmeasure.h>
#include <fstream>

#ifndef __clang__
#include <omp.h>
#endif

namespace inviwo {
using namespace discretedata;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo RawPercolationLoader::processorInfo_{
    "org.inviwo.RawPercolationLoader",  // Class identifier
    "Raw Percolation Loader",           // Display name
    "Percolation",                      // Category
    CodeState::Experimental,            // Code state
    Tags::None,                         // Tags
};

const ProcessorInfo RawPercolationLoader::getProcessorInfo() const { return processorInfo_; }

RawPercolationLoader::RawPercolationLoader()
    : Processor()
    , portOutData("OutData")
    , folderName("fileName", "File Name")
    , timeSlice("timeSlice", "Time Slice", 1, 1, 71)
    , fieldSize("fieldSize", "Field Size", size3_t(100), size3_t(0), size3_t(10000))
    , periodicX("periodicX", "X Periodic")
    , periodicY("periodicY", "Y Periodic")
    , periodicZ("periodicZ", "Z Periodic")
    , noHeader("noHeader", "Velocity without Header")
    , fullGrid("fullGrid", "All Vertex Postions given", true)
    , constVolume("constVolume", "Constant Volume", true) {
    addPort(portOutData);
    addProperty(folderName);
    addProperty(fieldSize);
    addProperty(timeSlice);
    addProperty(periodicX);
    addProperty(periodicY);
    addProperty(periodicZ);
    addProperty(noHeader);
    addProperty(fullGrid);
    addProperty(constVolume);

    folderName.setAcceptMode(AcceptMode::Open);
    folderName.setFileMode(FileMode::DirectoryOnly);
}

double* RawPercolationLoader::loadComponent(const std::string& filename,
                                            const ivec3 expectedFileSize, const ind header,
                                            double* pData, bool is2D) {
    // Load file
    if (!filesystem::fileExists(filename)) return nullptr;

    std::ifstream file(filename, std::ios::binary);

    ind dataSize = expectedFileSize.x * expectedFileSize.y;
    if (!is2D) {
        dataSize *= expectedFileSize.z;
    }

    // Get length of file.
    ind fileLength;
    file.seekg(0, std::ios::end);
    fileLength = file.tellg();
    file.seekg(0, std::ios::beg);

    // Find out header size.
    ind headerSize = 0;
    if (fileLength != static_cast<ind>(dataSize * sizeof(double))) {
        ind expectedNumber = dataSize * sizeof(double);
        ind header = 0;
        ind footer = 0;
        headerSize = 0;
        ind footerSize;
        for (footerSize = 4; footerSize <= 8; footerSize += 4) {

            file.seekg(fileLength - footerSize, std::ios::beg);
            file.read((char*)&footer, footerSize);
            if (footer == expectedNumber) {
                headerSize = fileLength - (dataSize * sizeof(double) + footerSize);
                if (headerSize > 0) {
                    file.seekg(0, std::ios::beg);
                    file.read((char*)&header, headerSize);

                    if (header != static_cast<ind>(dataSize * sizeof(double))) continue;
                }
                break;
            }
            headerSize = 0;
        }

        if (!headerSize) {
            file.seekg(0, std::ios::beg);
            file.read((char*)&header, headerSize);

            if (header != static_cast<ind>(dataSize * sizeof(double))) {
                std::cout << "Could not determine header size.\n";
                std::cout << "\tHeader size: " << headerSize << " - " << header << std::endl;
                std::cout << "\tFooter size: " << footerSize << " - " << footer << std::endl;
                return nullptr;
            }
        }
    }

    // We read less, if so desired. This way, we can read a subvolume.
    const ind BytesToBeRead = std::min((ind)fileLength, dataSize * (ind)sizeof(double));
    if (BytesToBeRead > fileLength || BytesToBeRead < 1) return nullptr;

    // Copy data into buffer
    char* buffer = pData ? (char*)pData : new char[BytesToBeRead];
    file.read(buffer, BytesToBeRead);

    // Check whether reading was successful
    if (!file) {
        if (!pData) delete[] buffer;
        file.close();
        return nullptr;
    }
    file.close();

    pData = reinterpret_cast<double*>(buffer);

    return pData;
}

namespace {
template <int N>
void interleaveData(std::shared_ptr<BufferChannel<double, N>> combined, double* data[N],
                    ind numElementsEach) {
    for (ind component = 0; component < N; ++component) {
        for (ind element = 0; element < numElementsEach; ++element) {
            combined->template get<typename std::array<double, N>>(element)[component] =
                data[component][element];
        }
    }
}

void explicitGridFromIntervals(double* intervals[3], size3_t dims,
                               std::shared_ptr<BufferChannel<double, 3>> dataOut) {

    for (size_t z = 0; z < dims.z; ++z)
        for (size_t y = 0; y < dims.y; ++y)
            for (size_t x = 0; x < dims.x; ++x) {
                ind lindex = x + dims.x * y + dims.x * dims.y * z;
                auto data = dataOut->get<glm::vec<3, double>>(lindex);
                data[0] = intervals[0][x];
                data[1] = intervals[1][y];
                data[2] = intervals[2][z];
            }
}
}  // namespace

DataSet* RawPercolationLoader::loadViaVectorComponents() {
    // Load file
    const std::string& Directory = folderName.get();
    if (!filesystem::directoryExists(Directory)) return nullptr;

    PerformanceTimer Timer;

    auto dims = fieldSize.get();
    const ind numElements = dims.x * dims.y * dims.z;
    const ind numStatElements = dims.x * dims.y;

    ind numZeros = 4 - (ind)std::log10(timeSlice.get());
    std::string zeros = std::string(numZeros, '0');
    zeros = zeros;

    ind headerSize = noHeader.get() ? 8 : 4;
    double* dataBuffer[3] = {
        loadComponent(Directory + "/VELOCITY/" + zeros + std::to_string(timeSlice.get()) + ".vx",
                      dims, 4),
        loadComponent(Directory + "/VELOCITY/" + zeros + std::to_string(timeSlice.get()) + ".vy",
                      dims, 4),
        loadComponent(Directory + "/VELOCITY/" + zeros + std::to_string(timeSlice.get()) + ".vz",
                      dims, 4)};

    double* gridBuffer[3] = {nullptr, nullptr, nullptr};
    if (!constVolume.get()) {
        gridBuffer[0] = loadComponent(Directory + "/VELOCITY/x", dims, headerSize);
        gridBuffer[1] = loadComponent(Directory + "/VELOCITY/y", dims, headerSize);
        gridBuffer[2] = loadComponent(Directory + "/VELOCITY/z", dims, headerSize);
    }

    double* avgBuffer[3] = {loadComponent(Directory + "/STAT/average_vx", dims, 4, nullptr, true),
                            loadComponent(Directory + "/STAT/average_vy", dims, 4, nullptr, true),
                            loadComponent(Directory + "/STAT/average_vz", dims, 4, nullptr, true)};

    if (!dataBuffer[0] || !dataBuffer[1] || !dataBuffer[2] || !avgBuffer[0] || !avgBuffer[1] ||
        !avgBuffer[2] ||
        (!constVolume.get() && (!gridBuffer[0] || !gridBuffer[1] || !gridBuffer[2]))) {
        LogInfo("Could not load all needed files.");
        for (int i = 0; i < 3; ++i) {
            delete[] dataBuffer[i];
            delete[] gridBuffer[i];
            delete[] avgBuffer[i];
        }
        return nullptr;
    }

    LogInfo("Raw data loading took " << Timer.ElapsedTime() << " seconds.");

    Timer.Reset();

    DataSet* dataSet = new DataSet(std::make_shared<PeriodicGrid<3>>(
        std::array<ind, 3>({(ind)dims.x, (ind)dims.y, (ind)dims.z}),
        std::array<bool, 3>({periodicX.get(), periodicY.get(), periodicZ.get()})));

    if (!constVolume.get()) {
        // Create grid buffer
        auto grid = std::make_shared<BufferChannel<double, 3>>(numElements, "Vertex Positions",
                                                               GridPrimitive::Vertex);
        dataSet->addChannel(grid);
        if (!constVolume.get()) {
            if (fullGrid.get())
                interleaveData<3>(grid, gridBuffer, numElements);
            else
                explicitGridFromIntervals(gridBuffer, dims, grid);
        }
    }

    // Compute percolation analysis scalar value
    auto AvgPercolationData = std::make_shared<BufferChannel<double, 3>>(
        numElements, "AveragedVelocity", GridPrimitive::Vertex);
    auto PercolationData =
        std::make_shared<BufferChannel<double, 3>>(numElements, "Velocity", GridPrimitive::Vertex);

    for (int n = 0; n < 3; ++n) {
#pragma omp parallel for
        for (ind linearIdx = 0; linearIdx < numElements; ++linearIdx) {
            // Raw data pointer to write to.
            auto& pData =
                (PercolationData->template get<typename std::array<double, 3>>(linearIdx));

            pData[n] = dataBuffer[n][linearIdx];
        }
    }

#pragma omp parallel for
    for (ind linearIdx = 0; linearIdx < numElements; ++linearIdx) {
        // Raw data pointer to write to.
        auto& pData = (AvgPercolationData->template get<typename std::array<double, 3>>(linearIdx));

        for (int n = 0; n < 3; ++n) {
            // Compute the percolation analysis scalar value
            double component =
                (dataBuffer[n][linearIdx] -
                 avgBuffer[n][linearIdx %
                              numStatElements]);  // / rmsBuffer[n][linearIdx % numStatElements];
            component = abs(component);
            pData[n] = std::isfinite(component) ? component : 0;
        }
    }
    LogInfo("\t\tGrid creation and normalization took " << Timer.ElapsedTime() << " seconds.");

    // Get rid of some memory
    for (int i = 0; i < 3; ++i) {
        delete[] dataBuffer[i];
        delete[] gridBuffer[i];
        delete[] avgBuffer[i];
    }
    dataSet->addChannel(PercolationData);
    dataSet->addChannel(AvgPercolationData);

    return dataSet;
}

void RawPercolationLoader::process() {
    if (data_ && !folderName.isModified() && !timeSlice.isModified() && !fieldSize.isModified() &&
        !noHeader.isModified()) {
        IVW_ASSERT(periodicX.isModified() || periodicY.isModified() || periodicZ.isModified(),
                   "Unexpected process() call without any changed property.");

        auto* perGrid = dynamic_cast<const PeriodicGrid<3>*>(data_->getGrid().get());
        IVW_ASSERT(perGrid != nullptr, "Assumed periodic grid.");

        auto newGrid = std::make_shared<PeriodicGrid<3>>(*perGrid);
        newGrid->setPeriodic(0, periodicX.get());
        newGrid->setPeriodic(1, periodicY.get());
        newGrid->setPeriodic(2, periodicZ.get());

        // Copy grid and data.
        auto newData = std::make_shared<DataSet>(newGrid);
        for (auto it = data_->cbegin(); it != data_->cend(); ++it) newData->addChannel(it->second);

        data_ = newData;
        portOutData.setData(data_);

        return;
    }
    LogInfo("=== Loading t=" << timeSlice.get() << " at " << fieldSize.get()[0] << ", "
                             << fieldSize.get()[1] << ", " << fieldSize.get()[2] << ":\n");

    PerformanceTimer Timer;
    data_ = std::shared_ptr<DataSet>(loadViaVectorComponents());
    if (!data_) {
        LogWarn("Loading failed.");
        return;
    }
    LogInfo("\tFile loading took " << Timer.ElapsedTime() << " seconds.");

    // Compute volume per voxel
    if (!constVolume.get()) {
        auto VolumeData = std::make_shared<BufferChannel<double, 1>>(
            data_->getGrid()->getNumElements(GridPrimitive::Volume), "Volume",
            GridPrimitive::Volume);
        double TotalVolume(0);

        auto Positions = data_->getChannel("Vertex Positions");

        for (auto element : data_->getGrid()->all(GridPrimitive::Volume)) {
            const double ThisVolume = euclidean::getMeasure(*Positions, element);
            VolumeData->get(element.getIndex()) = ThisVolume;
            TotalVolume += ThisVolume;
        }
        // dataSet->addChannel(VolumeData); We do not add this, since we do not want the
        // cell-based data.

        // Map volume from cell to vertices
        std::vector<ind> CellNeighs;
        auto VolumeDataVert = std::make_shared<BufferChannel<double>>(
            data_->getGrid()->getNumElements(GridPrimitive::Vertex), "Volume",
            GridPrimitive::Vertex);
        TotalVolume = 0;
        for (const auto& Vertex : data_->getGrid()->all(GridPrimitive::Vertex)) {
            // For all cells neighboring the vertex
            data_->getGrid()->getConnections(CellNeighs, Vertex.getIndex(), GridPrimitive::Vertex,
                                             GridPrimitive::Volume);

            // Each vertex gets an eigth of each neighboring cube
            double VertexVolume(0);
            for (const auto& Cell : CellNeighs) {
                VertexVolume += VolumeData->get(Cell) / 8.0;
            }

            // Set it
            VolumeDataVert->get(Vertex.getIndex()) = VertexVolume;
            TotalVolume += VertexVolume;
        }
        data_->addChannel(VolumeDataVert);
    } else {
        // Constant channel.
        auto VolumeDataVert = std::make_shared<AnalyticChannel<double, 1, double>>(
            [](double& val, ind) { val = 1.0; },
            data_->getGrid()->getNumElements(GridPrimitive::Vertex), "Volume",
            GridPrimitive::Vertex);
        data_->addChannel(VolumeDataVert);
    }

    portOutData.setData(data_);
}

}  // namespace inviwo
