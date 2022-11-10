/*********************************************************************
 *  Author  : Anke Friederici and Tino Weinkauf
 *  Init    : Thursday, January 25, 2018 - 15:56:16
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <percolation/percolationmoduledefine.h>

#include <combinatorialtopology/unionfind.h>
#include <inviwo/core/ports/dataoutport.h>
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/properties/datachannelproperty.h>
#include <modules/discretedata/connectivity/structuredgrid.h>
#include <inviwo/dataframe/datastructures/dataframe.h>

#include <random>

namespace inviwo {
using namespace discretedata;

/** \class PercolationAnalysis
    \brief Generate graphs of cluster properties
*/
class IVW_MODULE_PERCOLATION_API PercolationAnalysis : public Processor {
    // Friends
    // Types
public:
    /// Holds some infos between runs
    struct TStatCache {
        std::vector<float> largestCompVol;
        std::vector<float> totalCompVol;
        std::vector<float> normalizedCompVol;
        std::vector<int> numComps;
        std::vector<float> statH;
        std::vector<float> normalizedH;
        std::vector<int> RunID;
        std::vector<int> isPercolating;
        void clear() {
            largestCompVol.clear();
            totalCompVol.clear();
            normalizedCompVol.clear();
            numComps.clear();
            statH.clear();
            normalizedH.clear();
            isPercolating.clear();
            RunID.clear();
        }
    };

    enum PercolationDimension { X, Y, Z, ANY, ALL };

    // Construction / Deconstruction
public:
    PercolationAnalysis();
    virtual ~PercolationAnalysis() = default;

    // Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    /// Our main computation function
    virtual void process() override;

    template <typename T>
    void processChannel(const DataChannel<T, 1>& data, const DataChannel<double, 1>& volume,
                        const Connectivity& grid);
    struct Extent;
    void createClusterOutput(const UnionFind* clusters, const ind maxClusterId,
                             const std::map<ind, Extent>& extends,
                             const std::map<ind, double>& volumes,
                             const std::array<ind, 3>& totalSize);

    void updateProperties();
    template <typename T>
    void updatePropertiesByChannel(const DataChannel<T, 1>* data);

    // Ports
public:
    /// Data to be processed
    DataSetInport portInData;

    /// Output percolation statistics
    DataFrameOutport portOutTable;

    /// Dataset filled with output clusters
    DataSetOutport portOutClusters;

    /// Output cluster statistics (size)
    DataFrameOutport portOutClusterStatistics;

    // Properties
public:
    /// Scalar Field Channel worked upon
    DataChannelProperty propScalarChannel;

    /// Volume channel used to compute percolation function
    DataChannelProperty propVolumeChannel;

    /// How to choose min and max H
    CompositeProperty propMinMaxSettings;
    /// Take away some part of voxels from both ends
    BoolProperty propUsePercentage;
    /// Which percentage to take away from ends
    FloatProperty propPercentage;
    /// Whether to cut off both ends or not, available for comparison of super-level / sub-level
    /// sets
    BoolProperty propCutOffBothEnds;
    /// Min and max H values
    FloatMinMaxProperty propWindowH;

    /// How to choose min and max H
    CompositeProperty propSampleSettings;

    /// Value-based or Voxel-based
    OptionPropertyInt propSampleType;
    /// How often to sample the statistics
    IntProperty propNumSamples;
    /// Percolation dimension
    TemplateOptionProperty<PercolationDimension> propPercDim;

    /// To start an iteration over a parameter.
    ButtonProperty propIterationBtn;

    /// Everything related to analysing the performace / output of the algorithm
    CompositeProperty propAlgorithmAnalysis;

    /// Folder to write performance data to
    FileProperty propPerformanceStatsFolderName;

    /// All property regaring cluster output
    CompositeProperty propClusterOutput;

    /// Should the cluster statistics be output?
    BoolProperty propClusterStatsOutput;

    /// Sample Id at which to put out clusters
    IntSizeTProperty propSampleIdClusters;

    /// Already stop at that point
    BoolProperty propStopEarly;

    /// Threshold value at the sample id
    FloatProperty propThresholdValue;

    /// Record local/global clusters
    BoolProperty propLocalGlobalStats;

    /// Blocksize for parallelization
    IntSize3Property propBlockSize;

    /// Percentage of global clusters at selected sample
    FloatProperty propGlobalClusterPercentage;

    /// Percentage of global voxels at selected sample
    FloatProperty propGlobalVoxelPercentage;

    // Attributes
private:
    /// Keeps statistics between runs
    TStatCache StatCache;

    /// Run ID when iterating
    ind RunID;

protected:
    // Save statistics here
    using vec3i = glm::vec<3, ind>;
    struct Extent {
        vec3i min = {INT_MAX, INT_MAX, INT_MAX};
        vec3i max = {-1, -1, -1};

        Extent() {}
        Extent(const std::array<ind, 3>& pos) {
            for (ind dim = 0; dim < 3; ++dim) {
                min[dim] = pos[dim];
                max[dim] = pos[dim];
            }
        }
        void extend(const std::array<ind, 3>& pos) {
            for (ind dim = 0; dim < 3; ++dim) {
                min[dim] = std::min(min[dim], pos[dim]);
                max[dim] = std::max(max[dim], pos[dim]);
            }
        }

        void merge(const Extent& other) {
            for (ind dim = 0; dim < 3; ++dim) {
                min[dim] = std::min(min[dim], other.min[dim]);
                max[dim] = std::max(max[dim], other.max[dim]);
            }
        }

        bool isPercolating(const std::array<ind, 3>& size, const PercolationDimension& percDim) {
            if (percDim == PercolationDimension::X || percDim == PercolationDimension::Y ||
                percDim == PercolationDimension::Z)
                return (min[percDim] == 0 && max[percDim] == size[percDim] - 1);

            bool percolates = (percDim == PercolationDimension::ALL) ? true : false;

            for (ind dim = 0; dim < 3; ++dim) {
                if (size[dim] == 1) continue;
                bool percolatesDim = (min[dim] == 0 && max[dim] == size[dim] - 1);
                percolates = (percDim == PercolationDimension::ALL) ? (percolates && percolatesDim)
                                                                    : (percolates || percolatesDim);
            }
            return percolates;
        }
    };
};

template <typename T>
void PercolationAnalysis::processChannel(const DataChannel<T, 1>& data,
                                         const DataChannel<double, 1>& volume,
                                         const Connectivity& grid) {
    ivwAssert(data.getGridPrimitiveType() == volume.getGridPrimitiveType(),
              "Data and volume must be given on same grid element.");

    ind NumVertices = data.size();

    // Sort by value
    std::vector<std::pair<T, ind>> values(NumVertices, std::make_pair((T)0, -1));
    for (ind dIdx = 0; dIdx < NumVertices; ++dIdx) {
        values[dIdx].second = dIdx;
        data.fill(values[dIdx].first, dIdx);
    }
    std::sort(values.begin(), values.end(), [](const auto& a, const auto& b) {
        return (a.first == b.first) ? (a.second > b.second) : (a.first > b.first);
    });

    // Excude -inf values (These are created for exlusion of borders in the Duct dataset case).
    auto endBound = std::lower_bound(values.begin(), values.end(),
                                     static_cast<T>(-std::numeric_limits<double>::max()),
                                     [](auto a, auto b) { return a.first > b; });
    // Should we increase the endBound by one?
    ind NumElements = endBound - values.begin();

    ind minIdx = 0;
    ind maxIdx = NumElements - 1;
    ind numSamples = propNumSamples.get();
    float minVal, maxVal;

    // Take percentage of data away.
    if (propUsePercentage.get()) {
        minIdx = std::floor((float)NumElements * propPercentage.get() * 0.01f);
        minIdx = std::max(ind(0), minIdx);

        if (propCutOffBothEnds.get()) {
            maxIdx = std::ceil((float)NumElements * (100.0f - propPercentage.get()) * 0.01f);
            maxIdx = std::min(NumElements - 1, maxIdx);
        }

        // Exclude all that have the same value as well, hoever this creates problem with sample
        // number / positions
        // while (minIdx > 0 && values[minIdx].first == values[minIdx - 1].first)
        // minIdx--; while (maxIdx < NumElements - 1 && values[maxIdx].first == values[maxIdx +
        // 1].first)
        //     maxIdx++;

        minVal = values[maxIdx].first;
        maxVal = values[minIdx].first;

        LogInfo("Data within range = [" << values[maxIdx].first << "(" << maxIdx << "), "
                                        << values[minIdx].first << "(" << minIdx << ")]");
    } else {
        // Look for min and max value.
        // For upper bound (compare (value element)), for lower bound (compare(element, value))
        minIdx = std::upper_bound(values.begin(), endBound, propWindowH.getEnd(),
                                  [](auto a, auto b) { return a > b.first; }) -
                 values.begin();
        maxIdx = std::lower_bound(values.begin(), endBound, propWindowH.getStart(),
                                  [](auto a, auto b) { return a.first > b; }) -
                 values.begin();

        minIdx = std::max(ind(0), minIdx);
        maxIdx = std::min(NumElements - 1, maxIdx);

        minVal = propWindowH.getStart();
        maxVal = propWindowH.getEnd();

        LogInfo("Data within range = [" << values[maxIdx].first << "(" << maxIdx << "), "
                                        << values[minIdx].first << "(" << minIdx << ")]");
    }

    // Update after filtering
    NumElements = maxIdx - minIdx + 1;

    // Step size in case of non-uniform sampling.
    double hStep = -1;  // Actual H value in the data
    ind binSize = std::max(((NumElements - 1) / (numSamples - 1)), ind(1));
    // Value-based sampling
    if (propSampleType.get() == 0) {
        hStep = double(maxVal - minVal) / (numSamples - 1);
        // Voxel-based sampling
    } else {
        numSamples = (NumElements - 1) / binSize + 1;
    }

    // Save statistics here
    std::map<ind, double> VolumePerComponent;
    std::map<ind, Extent> ExtentPerComponent;
    double TotalVolume = 0;

    // - memory concerns
    const ind PreviousStatCacheSize = (ind)StatCache.statH.size();
    StatCache.largestCompVol.reserve(StatCache.largestCompVol.size() + numSamples);
    StatCache.totalCompVol.reserve(StatCache.totalCompVol.size() + numSamples);
    StatCache.normalizedCompVol.reserve(StatCache.normalizedCompVol.size() + numSamples);
    StatCache.numComps.reserve(StatCache.numComps.size() + numSamples);
    StatCache.statH.reserve(StatCache.statH.size() + numSamples);
    StatCache.normalizedH.reserve(StatCache.normalizedH.size() + numSamples);
    StatCache.RunID.reserve(StatCache.RunID.size() + numSamples);
    StatCache.isPercolating.reserve(StatCache.isPercolating.size() + numSamples);

    double maxVolume = 0;
    ind maxVolumeIndex = -2;
    bool percolating = false;

    // Structured grid? Use to find out if percolating.
    const StructuredGrid<3>* lattice = dynamic_cast<const StructuredGrid<3>*>(&grid);
    std::array<ind, 3> latticeVertSize, idxVec;
    if (lattice) {
        latticeVertSize = lattice->getNumVertices();
    }

    // Dimensionality of the grid elements
    GridPrimitive GridElemDim = data.getGridPrimitiveType();

    // Setup union-find
    UnionFind UF(NumVertices);

    std::vector<ind> Neighbors;
    int numMerges = 0;
    int numCreates = 0;
    int numExtends = 0;

    //
    std::vector<double> xValuesStat;
    double nextVal = maxVal;

    // Run over all grid elements in decreasing order
    for (ind i(0); i <= maxIdx; i++) {
        // Shorthand
        const std::pair<T, ind>& Current = values[i];
        double CurrentVolume;
        volume.fill(CurrentVolume, Current.second);
        TotalVolume += CurrentVolume;

        if (lattice) {
            idxVec = StructuredGrid<3>::indexFromLinear(Current.second, latticeVertSize);
        }

        // Get the number of components in the neighborhood of this grid element
        // - get a neighborhood iterator with same dimensionality
        grid.getConnections(Neighbors, Current.second, GridElemDim, GridElemDim);
        // - for each neighbor
        std::set<ind> NeighComps;
        for (const ind& idNeigh : Neighbors) {
            const ind idSet = UF.Find(idNeigh);
            if (idSet >= 0) NeighComps.insert(idSet);
        }

        // Create, extend, or merge components based on the number of components that we have in
        // the neighborhood
        switch (NeighComps.size()) {
            case 0: {
                numCreates++;

                UF.MakeSet(Current.second);
                VolumePerComponent.insert(std::make_pair(Current.second, CurrentVolume));

                // Update maxima.
                if (CurrentVolume > maxVolume) {
                    maxVolume = CurrentVolume;
                    maxVolumeIndex = Current.second;
                }
                maxVolume = std::max(maxVolume, CurrentVolume);
                if (lattice)
                    ExtentPerComponent.insert(std::make_pair(Current.second, Extent(idxVec)));
                break;
            }

            case 1: {
                numExtends++;
                const ind ExtendID = *(NeighComps.cbegin());
                UF.ExtendSetByID(ExtendID, Current.second);
                VolumePerComponent[ExtendID] += CurrentVolume;

                double newVolume = VolumePerComponent[ExtendID];

                // Update maxima.
                if (newVolume > maxVolume) {
                    maxVolume = newVolume;
                    maxVolumeIndex = ExtendID;
                }

                if (lattice) {
                    ExtentPerComponent[ExtendID].extend(idxVec);
                    if (ExtentPerComponent[ExtendID].isPercolating(latticeVertSize,
                                                                   propPercDim.getSelectedValue()))
                        percolating = true;
                }
                break;
            }

            default: {
                numMerges++;

                // We have more than 1 component. All of them need to be merged.
                // In our specific case, it does not matter which component "wins".
                // - get the first element
                auto it = NeighComps.cbegin();
                const ind FirstComp = *it;
                for (it++; it != NeighComps.cend(); it++) {
                    UF.Union(*it, FirstComp);
                    VolumePerComponent[FirstComp] += VolumePerComponent[*it];

                    // Merge extents.
                    if (lattice) ExtentPerComponent[FirstComp].merge(ExtentPerComponent[*it]);

                    VolumePerComponent.erase(*it);
                }
                // - and the current point itself!
                UF.ExtendSetByID(FirstComp, Current.second);
                VolumePerComponent[FirstComp] += CurrentVolume;

                double newVolume = VolumePerComponent[FirstComp];

                // Update maxima.
                if (newVolume > maxVolume) {
                    maxVolume = newVolume;
                    maxVolumeIndex = FirstComp;
                }

                maxVolume = std::max(maxVolume, VolumePerComponent[FirstComp]);
                if (lattice) {
                    ExtentPerComponent[FirstComp].extend(idxVec);
                    if (ExtentPerComponent[FirstComp].isPercolating(latticeVertSize,
                                                                    propPercDim.getSelectedValue()))
                        percolating = true;
                }
                break;
            }
        }

        if (i < minIdx) continue;

        ind numInStatWindow = 0;
        double xValue = values[i].first;
        xValuesStat.clear();

        // Find out if we need to write a sample.
        if (propSampleType.get() == 1) {
            // Sample equal bins, given bin size.
            numInStatWindow = ((i - minIdx) % binSize == 0) ? 1 : 0;
            // Value-based sample: We repeat samples when values do not occur
        } else {
            while (xValue < nextVal) {
                numInStatWindow++;
                xValuesStat.push_back(nextVal);
                nextVal -= hStep;
            }
        }

        // Always include the final index
        if (i == maxIdx && !numInStatWindow) {
            numInStatWindow = std::max(ind(1), numInStatWindow);
            if (propSampleType.get() == 0) xValuesStat.push_back(minVal);
        }

        // Select an h value.
        if (numInStatWindow && propSampleType.get() == 1) xValuesStat.push_back(xValue);

        bool createdOutput = false;

        // Record statistics
        for (ind copyBin = 0; copyBin < numInStatWindow; ++copyBin) {
            if (propClusterStatsOutput.get() &&
                StatCache.statH.size() - PreviousStatCacheSize == propSampleIdClusters.get()) {
                createClusterOutput(&UF, maxVolumeIndex, ExtentPerComponent, VolumePerComponent,
                                    latticeVertSize);
                propThresholdValue.set(xValuesStat[copyBin]);
                createdOutput = true;
            }
            StatCache.RunID.push_back(RunID);
            StatCache.statH.push_back(xValuesStat[copyBin]);
            double normH = 1.0 - (xValuesStat[copyBin] - minVal) / (maxVal - minVal);
            StatCache.normalizedH.push_back(normH);
            StatCache.numComps.push_back((int)UF.GetNumSets());
            double normVolume = (float)TotalVolume / NumVertices;
            StatCache.normalizedCompVol.push_back(normVolume);
            StatCache.totalCompVol.push_back((float)TotalVolume);
            const double LargestCompVol = maxVolume;
            StatCache.largestCompVol.push_back((float)LargestCompVol);
            StatCache.isPercolating.push_back(percolating ? 1 : 0);
        }

        if (propStopEarly.get() && createdOutput) break;
    }

    // Prepare output data
    auto pOutTable = std::make_shared<DataFrame>();
    // - How many rows we have
    const ind NumStatsRows = (ind)StatCache.statH.size();

    // Add columns
    auto pIterID = pOutTable->addColumn<int>("Iteration", NumStatsRows);
    auto& IterID = pIterID->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
    // --
    auto pStatH = pOutTable->addColumn<float>("H", NumStatsRows);
    auto& StatH = pStatH->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
    // --
    auto pNormalizedH = pOutTable->addColumn<float>("Value Fraction", NumStatsRows);
    auto& NormalizedH =
        pNormalizedH->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
    // --
    auto pStatNormVol = pOutTable->addColumn<float>("Normalized Volume", NumStatsRows);
    auto& NormVol =
        pStatNormVol->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
    // --
    auto pStatNumComp = pOutTable->addColumn<int>("Number of connected components", NumStatsRows);
    auto& AllComp =
        pStatNumComp->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
    // --
    auto pStatMaxComp =
        pOutTable->addColumn<int>("Maximum number of connected components", NumStatsRows);
    auto& MaxComp =
        pStatMaxComp->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
    // --
    auto pStatNumCompRatio = pOutTable->addColumn<float>(
        "Number of connected components / Maximum number of connected components", NumStatsRows);
    auto& CompRatio =
        pStatNumCompRatio->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
    // --
    auto pStatLargestCompVol =
        pOutTable->addColumn<float>("Volume largest connected component", NumStatsRows);
    auto& VolLargest =
        pStatLargestCompVol->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
    // --
    auto pStatTotalVol = pOutTable->addColumn<float>("Total Volume", NumStatsRows);
    auto& VolTotal =
        pStatTotalVol->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
    // --
    auto pStatVolumeRatio =
        pOutTable->addColumn<float>("Largest volume / Total volume", NumStatsRows);
    auto& VolRatio =
        pStatVolumeRatio->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();

    // --
    auto pIsPercolating = pOutTable->addColumn<int>(
        "Is percolating " + propPercDim.getSelectedDisplayName(), NumStatsRows);
    auto& PercolatingState =
        pIsPercolating->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();

    // Fill table
    if (NumStatsRows > 0) {
        auto itStartThisRun = StatCache.numComps.cbegin();
        std::advance(itStartThisRun, PreviousStatCacheSize);
        const int MaxNumConnectedComponents =
            *(std::max_element(itStartThisRun, StatCache.numComps.cend()));

        for (ind i(0); i < NumStatsRows; i++) {
            IterID[i] = StatCache.RunID[i];
            StatH[i] = StatCache.statH[i];
            NormalizedH[i] = StatCache.normalizedH[i];
            NormVol[i] = StatCache.normalizedCompVol[i];
            AllComp[i] = StatCache.numComps[i];
            CompRatio[i] = float(StatCache.numComps[i]) / float(MaxNumConnectedComponents);
            MaxComp[i] = MaxNumConnectedComponents;
            VolLargest[i] = StatCache.largestCompVol[i];
            VolTotal[i] = StatCache.totalCompVol[i];
            VolRatio[i] = StatCache.largestCompVol[i] / StatCache.totalCompVol[i];
            PercolatingState[i] = StatCache.isPercolating[i];
        }
    }

    pOutTable->updateIndexBuffer();
    // Throw out the data
    portOutTable.setData(pOutTable);

}  // namespace inviwo

inline void PercolationAnalysis::createClusterOutput(const UnionFind* clusters,
                                                     const ind maxClusterId,
                                                     const std::map<ind, Extent>& extends,
                                                     const std::map<ind, double>& volumes,
                                                     const std::array<ind, 3>& totalSize) {

    auto pInDataSet = portInData.getData();
    ind NumVertices = propScalarChannel.getCurrentChannel()->size();

    // Channel for marking clusters
    auto outData = std::make_shared<DataSet>(*pInDataSet.get());
    auto largestClusterChannel = std::make_shared<BufferChannel<float, 1>>(
        NumVertices, "Largest Cluster", GridPrimitive::Vertex);
    auto allClustersChannel = std::make_shared<BufferChannel<float, 1>>(NumVertices, "All Clusters",
                                                                        GridPrimitive::Vertex);
    auto clusterIdChannel = std::make_shared<BufferChannel<float, 1>>(NumVertices, "Clusters Ids",
                                                                      GridPrimitive::Vertex);

    std::shared_ptr<BufferChannel<float, 1>> localGLobalClusterChannel;
    std::shared_ptr<BufferChannel<float, 1>> distributionTypeChannel;
    if (propLocalGlobalStats.get()) {
        localGLobalClusterChannel = std::make_shared<BufferChannel<float, 1>>(
            NumVertices, "Local/GLobal Clusters", GridPrimitive::Vertex);
        distributionTypeChannel = std::make_shared<BufferChannel<float, 1>>(
            NumVertices, "Distribution Type", GridPrimitive::Vertex);
    }

    // Set of all current clusters ids
    std::map<ind, bool> clusterIdsLocal;

    clusterIdsLocal[-1] = false;

    bool isLocal = true;
    std::vector<ind> idxVec;

    ind numLocalClusters = 0;
    ind numGlobalClusters = 0;

    ind numLocalVoxels = 0;
    ind numGlobalVoxels = 0;

    size3_t blockSize = propBlockSize.get();
    for (ind dIdx = 0; dIdx < NumVertices; ++dIdx) {
        ind clusterId = clusters->Find(dIdx);

        // Already in the map? (If not, add)
        if (propLocalGlobalStats.get()) {
            auto found = clusterIdsLocal.find(clusterId);
            if (found == clusterIdsLocal.end()) {
                // Check if the cluster is local
                size3_t blockIdxLower;
                size3_t blockIdxUpper;
                const Extent& extent = extends.find(clusterId)->second;

                // Fully within one block?
                blockIdxUpper = size3_t(extent.max[0] / blockSize[0], extent.max[1] / blockSize[1],
                                        extent.max[2] / blockSize[2]);
                blockIdxLower = size3_t(extent.min[0] / blockSize[0], extent.min[1] / blockSize[1],
                                        extent.min[2] / blockSize[2]);

                if (blockIdxLower == blockIdxUpper) {
                    // Check if any coordinate on upper/lower bound
                    isLocal = true;

                    size3_t blockLowerBound = blockIdxLower * blockSize;
                    size3_t blockUpperBound = blockLowerBound + blockSize;

                    // Mark local part of the block
                    for (ind dim = 0; dim < 3; ++dim) {
                        // Low side.
                        if (blockLowerBound[dim] > 0) blockLowerBound[dim]++;
                        // High side.
                        if (static_cast<ind>(blockUpperBound[dim]) < totalSize[dim])
                            blockUpperBound[dim] -= 2;
                    }

                    // Are both upper and lower in the local part
                    for (ind dim = 0; dim < 3; ++dim) {
                        // Low side.
                        if (static_cast<ind>(blockLowerBound[dim]) > extent.min[dim]) {
                            isLocal = false;
                            break;
                        }

                        // High side.
                        if (static_cast<ind>(blockUpperBound[dim]) < extent.max[dim]) {
                            isLocal = false;
                            break;
                        }
                    }

                } else {
                    isLocal = false;
                }

                // Add to map
                clusterIdsLocal[clusterId] = isLocal;

                if (isLocal) {
                    numLocalClusters++;
                } else {
                    numGlobalClusters++;
                }
            } else {
                isLocal = found->second;
            }

            localGLobalClusterChannel->get(dIdx) = clusterId < 0 ? 0.0f : (isLocal ? -1.0f : 1.0f);

            if (clusterId >= 0) {
                numGlobalVoxels += isLocal ? 0 : 1;
                numLocalVoxels += isLocal ? 1 : 0;
            }

            bool isPosLocal = true;
            std::array<ind, 3> vecIdx = StructuredGrid<3>::indexFromLinear(dIdx, totalSize);
            size3_t blockIdxLower = size3_t(vecIdx[0] / blockSize[0], vecIdx[1] / blockSize[1],
                                            vecIdx[2] / blockSize[2]);
            size3_t blockLowerBound = blockIdxLower * blockSize;
            size3_t blockUpperBound = blockLowerBound + blockSize;

            // Mark local part of the block
            for (ind dim = 0; dim < 3; ++dim) {
                // Low side.
                if (blockLowerBound[dim] > 0) blockLowerBound[dim]++;
                if (static_cast<ind>(blockLowerBound[dim]) > vecIdx[dim]) {
                    isPosLocal = false;
                    break;
                }
                // High side.
                if (static_cast<ind>(blockUpperBound[dim]) < totalSize[dim])
                    blockUpperBound[dim] -= 2;
                if (static_cast<ind>(blockUpperBound[dim]) < vecIdx[dim]) {
                    isPosLocal = false;
                    break;
                }
            }

            distributionTypeChannel->get(dIdx) = (isPosLocal ? -1.0f : 1.0f);
        }

        largestClusterChannel->get(dIdx) = clusterId == maxClusterId ? 1.0f : 0.0f;
        allClustersChannel->get(dIdx) =
            clusterId < 0 ? 0.0f : (clusterId == maxClusterId ? -1.0f : 1.0f);
        clusterIdChannel->get(dIdx) = static_cast<float>(clusterId);
    }

    outData->addChannel(largestClusterChannel);
    outData->addChannel(allClustersChannel);
    outData->addChannel(clusterIdChannel);
    if (propLocalGlobalStats.get()) {
        outData->addChannel(localGLobalClusterChannel);
        propGlobalClusterPercentage.set(100.0f * static_cast<float>(numGlobalClusters) /
                                        (numGlobalClusters + numLocalClusters));
        propGlobalVoxelPercentage.set(100.0f * static_cast<float>(numGlobalVoxels) /
                                      (numGlobalVoxels + numLocalVoxels));
        outData->addChannel(distributionTypeChannel);
    }

    portOutClusters.setData(outData);

    if (propClusterStatsOutput.get()) {

        // Setup dataframe for cluster stats
        auto pOutClusterStats = std::make_shared<DataFrame>();
        // - How many rows we have
        const ind NumStatsRows = (ind)clusterIdsLocal.size();

        // Add columns
        auto pClusterIds = pOutClusterStats->addColumn<int>("Cluseter Id", NumStatsRows);
        auto& clusterIds =
            pClusterIds->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
        // --
        auto pVolume = pOutClusterStats->addColumn<float>("Volume", NumStatsRows);
        auto& volume =
            pVolume->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
        // --
        auto pSizeX = pOutClusterStats->addColumn<int>("Size X", NumStatsRows);
        auto& sizeX = pSizeX->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
        // --
        auto pSizeY = pOutClusterStats->addColumn<int>("Size Y", NumStatsRows);
        auto& sizeY = pSizeY->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
        // --
        auto pSizeZ = pOutClusterStats->addColumn<int>("Size Z", NumStatsRows);
        auto& sizeZ = pSizeZ->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();
        // --
        auto pSizeBox = pOutClusterStats->addColumn<int>("Size BBox", NumStatsRows);
        auto& sizeBox =
            pSizeBox->getTypedBuffer()->getEditableRAMRepresentation()->getDataContainer();

        ind statIndex = 0;
        for (auto pair : clusterIdsLocal) {
            ind clusterId = pair.first;
            if (clusterId < 0) continue;
            clusterIds[statIndex] = static_cast<int>(clusterId);
            volume[statIndex] = static_cast<float>(volumes.find(clusterId)->second);
            const Extent& extend = extends.find(clusterId)->second;
            sizeX[statIndex] = static_cast<int>(extend.max[0] - extend.min[0] + 1);
            sizeY[statIndex] = static_cast<int>(extend.max[1] - extend.min[1] + 1);
            sizeZ[statIndex] = static_cast<int>(extend.max[2] - extend.min[2] + 1);
            sizeBox[statIndex] = sizeX[statIndex] * sizeY[statIndex] * sizeZ[statIndex];
            statIndex++;
        }

        portOutClusterStatistics.setData(pOutClusterStats);
    }
}

inline void PercolationAnalysis::updateProperties() {
    auto Data = propScalarChannel.getCurrentChannel();
    if (!Data || Data->getNumComponents() != 1) {
        return;
    }
    Data->dispatch<void, dispatching::filter::Scalars, 1, 1>(
        [&](auto channel) { updatePropertiesByChannel(channel); });
}

template <typename T>
void PercolationAnalysis::updatePropertiesByChannel(const DataChannel<T, 1>* data) {

    T minT, maxT;
    data->getMin(minT);
    data->getMax(maxT);

    float min = std::max(std::numeric_limits<float>::lowest(), static_cast<float>(minT));
    float max = std::min(std::numeric_limits<float>::max(), static_cast<float>(maxT));

    float oldStart = propWindowH.getStart();
    float oldEnd = propWindowH.getEnd();

    propWindowH.setRangeMin(min);
    propWindowH.setRangeMax(max);

    propThresholdValue.setMinValue(min);
    propThresholdValue.setMaxValue(max);

    if (oldStart < min || oldStart > max || oldEnd < min || oldEnd > max) {
        propWindowH.setStart(min);
        propWindowH.setEnd(max);
    }

    if (propUsePercentage.get()) {
        propPercentage.setReadOnly(false);
        propWindowH.setReadOnly(true);
    } else {
        propPercentage.setReadOnly(true);
        propWindowH.setReadOnly(false);
    }
}

}  // namespace inviwo
