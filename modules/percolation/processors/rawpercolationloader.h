/*********************************************************************
 *  Author  : Anke Friederici & Tino Weinkauf
 *  Init    : Tuesday, February 20, 2018 - 23:41:57
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <percolation/percolationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <modules/discretedata/ports/datasetport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/boolproperty.h>

namespace inviwo {
using namespace discretedata;

/** \class RawPercolationLoader
    \brief Load a dataset generated from Nek5000.
    Manually insert folder and 3D sizes.

    @author Anke Friederici & Tino Weinkauf
*/
class IVW_MODULE_PERCOLATION_API RawPercolationLoader : public Processor {
    // Friends
    // Types
public:
    // Construction / Deconstruction
public:
    RawPercolationLoader();
    virtual ~RawPercolationLoader() = default;

    // Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    /// Our main computation function
    virtual void process() override;

    DataSet* loadViaVectorComponents();

public:
    static double* loadComponent(const std::string& filename, const ivec3 expectedFileSize,
                                 const ind header = 4, double* pData = nullptr, bool is2D = false);

    // Ports
public:
    /// Data to be generated
    DataSetOutport portOutData;

    // Properties
public:
    /// File name
    FileProperty folderName;
    IntProperty timeSlice;

    /// Field size
    IntSize3Property fieldSize;
    BoolProperty periodicX, periodicY, periodicZ, noHeader, fullGrid, constVolume;

    // Attributes
private:
    std::shared_ptr<DataSet> data_;
};

}  // namespace
