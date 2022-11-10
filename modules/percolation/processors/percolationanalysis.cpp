/*********************************************************************
 *  Author  : Anke Friederici and Tino Weinkauf
 *  Init    : Thursday, January 25, 2018 - 15:56:16
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <percolation/processors/percolationanalysis.h>

#include <inviwo/core/util/filesystem.h>
#include <modules/discretedata/connectivity/periodicgrid.h>
#include <modules/discretedata/dataset.h>
#include <modules/kxtools/performancetimer.h>

namespace inviwo {
using namespace discretedata;

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PercolationAnalysis::processorInfo_{
    "org.inviwo.PercolationAnalysis",  // Class identifier
    "Percolation Analysis",            // Display name
    "Percolation",                     // Category
    CodeState::Experimental,           // Code state
    Tags::None,                        // Tags
};

const ProcessorInfo PercolationAnalysis::getProcessorInfo() const { return processorInfo_; }

PercolationAnalysis::PercolationAnalysis()
    : Processor()
    , portInData("InData")
    , portOutTable("OutTable")
    , portOutClusters("OutClusters")
    , portOutClusterStatistics("OutClusterStatistics")
    , propScalarChannel(portInData, "ScalarChannel", "Scalar",
                        [](const std::shared_ptr<const Channel> a) {
                            return (a->getGridPrimitiveType() == GridPrimitive::Vertex &&
                                    a->getNumComponents() == 1);
                        })
    , propVolumeChannel(portInData, "VolumeChannel", "Volume",
                        [](const std::shared_ptr<const Channel> a) {
                            return (a->getGridPrimitiveType() == GridPrimitive::Vertex &&
                                    a->getNumComponents() == 1);
                        })
    /// How to set H range
    , propMinMaxSettings("minMaxSettings", "Range of H")
    , propUsePercentage("usePercentage", "Percentage-based")
    , propPercentage("percentage", "Cut-Off from Ends", 0, 0, 10, 0.5)
    , propCutOffBothEnds("cutOffBothEnds", "Both Ends", false)
    , propWindowH("WindowH", "H Window", 1, 2, 0, 3)

    /// How to sample H
    , propSampleSettings("sampleSettings", "Sampling of H")
    , propSampleType("sampleType", "Sampling Type")
    , propNumSamples("numSamples", "Num Samples", 100, 1, 10000000)
    , propPercDim("percDim", "Percolation Dimension",
                  {{"dimX", "X", PercolationDimension::X},
                   {"dimY", "Y", PercolationDimension::Y},
                   {"dimZ", "Z", PercolationDimension::Z},
                   {"dimAny", "Any", PercolationDimension::ANY},
                   {"dimAll", "All", PercolationDimension::ALL}})

    // Iteration
    , propIterationBtn("IterationBtn", "Iterate", InvalidationLevel::Valid)
    , propAlgorithmAnalysis("algorithmAnalysis", "Algorithm Analysis")
    , propPerformanceStatsFolderName("statFolder", "Statistics Folder")

    // Cluster Ids output
    , propClusterOutput("clusterOutput", "Cluster Output")
    , propClusterStatsOutput("clusterStatsOutput", "Stats Output", false)
    , propSampleIdClusters("sampleId", "Sample Index", 100, 0, 10000000)
    , propStopEarly("stopEarly", "Stop Early", false)
    , propThresholdValue("thresholdValue", "Threshold Value")
    , propLocalGlobalStats("distributedStats", "Distribution Stats", false)
    , propBlockSize("blockSize", "Block Size", vec3(100))
    , propGlobalClusterPercentage("globalClusterPercentage", "Global Cluster Fraction", 0.0f, 0.0f,
                                  100.f, 0.1f)
    , propGlobalVoxelPercentage("globalVoxelPercentage", "Global Voxel Fraction", 0.0f, 0.0f, 100.f,
                                0.1f) {

    addPort(portInData);
    addPort(portOutTable);
    addPort(portOutClusters);
    addPort(portOutClusterStatistics);

    addProperty(propScalarChannel);
    addProperty(propVolumeChannel);

    addProperty(propPerformanceStatsFolderName);

    // H range
    addProperty(propMinMaxSettings);
    // propWindowH.setSemantics(PropertySemantics::Text);
    propMinMaxSettings.addProperties(propUsePercentage, propPercentage, propCutOffBothEnds,
                                     propWindowH);

    // H sampling
    addProperty(propSampleSettings);
    propNumSamples.setSemantics(PropertySemantics::Text);
    propNumSamples.setCurrentStateAsDefault();
    propSampleSettings.addProperties(propSampleType, propNumSamples);
    propSampleType.addOption("valueBased", "Value-Based", 0);
    propSampleType.addOption("voxelBased", "Voxel-Based", 1);

    propPerformanceStatsFolderName.setAcceptMode(AcceptMode::Open);
    propPerformanceStatsFolderName.setFileMode(FileMode::DirectoryOnly);

    RunID = -1;  // We are not iterating
    propIterationBtn.onChange([&]() {
        if (RunID < 0) {
            // Prepare for iteration
            RunID = 0;
            StatCache.clear();

            propIterationBtn.setDisplayName("Iterating...  Press to Stop");
        } else {
            // Stop iteration
            RunID = -1;
            propIterationBtn.setDisplayName("Iterate");
        }
    });

    addProperties(propPercDim, propIterationBtn);

    addProperty(propAlgorithmAnalysis);
    propAlgorithmAnalysis.addProperties(propClusterOutput);

    propClusterOutput.addProperties(propClusterStatsOutput, propSampleIdClusters,
                                    propThresholdValue, propStopEarly, propLocalGlobalStats,
                                    propBlockSize, propGlobalClusterPercentage,
                                    propGlobalVoxelPercentage);

    propThresholdValue.setReadOnly(true);

    propBlockSize.visibilityDependsOn(propLocalGlobalStats, [](auto& p) { return p.get(); });
    propGlobalClusterPercentage.visibilityDependsOn(propLocalGlobalStats,
                                                    [](auto& p) { return p.get(); });
    propGlobalClusterPercentage.setReadOnly(true);
    propGlobalVoxelPercentage.visibilityDependsOn(propLocalGlobalStats,
                                                  [](auto& p) { return p.get(); });
    propGlobalVoxelPercentage.setReadOnly(true);
    propNumSamples.onChange([&]() { propSampleIdClusters.setMaxValue(propNumSamples.get()); });

    updateProperties();
}

void PercolationAnalysis::process() {
    // Get data
    auto pInDataSet = portInData.getData();
    if (!pInDataSet || pInDataSet->getNumChannels() < 2) {
        LogWarn("Need at least 2 channels.");
        return;
    }

    // Get desired channel
    auto Data = propScalarChannel.getCurrentChannel();
    if (!Data || Data->getNumComponents() != 1) {
        LogWarn("Could not load the percolation scalar.");
        return;
    }

    auto InVolume = propVolumeChannel.getCurrentChannel();
    if (!InVolume || InVolume->getNumComponents() != 1) {
        LogWarn("Could not load cell volume data.");
        return;
    }

    updateProperties();

    ivwAssert(pInDataSet->getGrid(), "No grid given");

    std::shared_ptr<const DataChannel<double, 1>> Volume =
        std::dynamic_pointer_cast<const DataChannel<double, 1>, const Channel>(InVolume);
    if (!Volume) return;

    // Accumulate statistics?
    if (RunID < 0) {
        // No, not iterating
        StatCache.clear();
    } else {
        // Yes, we are iterating
        RunID++;
    }

    PerformanceTimer Timer;

    Data->dispatch<void, dispatching::filter::Scalars, 1, 1>(
        [&](auto channel) { this->processChannel(*channel, *Volume, *(pInDataSet->getGrid())); });

    float timey = Timer.ElapsedTime();
    LogInfo("\tStatistic creation took " << timey << " seconds.");

    // Record performance, if desired by user
    if (filesystem::directoryExists(propPerformanceStatsFolderName.get())) {
        // LogProcessorInfo("Dir exists.");
        std::ofstream statFile;
        statFile.open(propPerformanceStatsFolderName.get() + "Performance.csv",
                      std::ios::out | std::ios::app);

        if (statFile.is_open()) {
            const auto* strucGrid =
                dynamic_cast<const StructuredGrid<3>*>(pInDataSet->getGrid().get());
            statFile << strucGrid->getNumVerticesInDimension(0) << ','
                     << strucGrid->getNumVerticesInDimension(1) << ','
                     << strucGrid->getNumVerticesInDimension(2) << ',' << timey << '\n';
            statFile.close();
        }
    }
}

}  // namespace inviwo
