/*********************************************************************
 *  Author  : Anke Friederici & Tino Weinkauf
 *  Init    : Tuesday, April 24, 2018 - 13:34:10
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
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {
using namespace discretedata;

/** \class ScalarTransform
    \brief Compress data to one scalar.

    @author Anke Friederici & Tino Weinkauf
*/
class IVW_MODULE_PERCOLATION_API ScalarTransform : public Processor {
    // Friends
    // Types
public:
    // Construction / Deconstruction
public:
    ScalarTransform();
    virtual ~ScalarTransform() = default;

    // Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    /// Our main computation function
    virtual void process() override;

    void OnChangeRMS();

    // Ports
public:
    /// Data to be processed
    DataSetInport portInData;

    /// Data to be generated
    DataSetOutport portOutData;

    // Properties
public:
    /// File name
    FileProperty rmsName;
    /// Setting scalar to -inf where RMS is 0
    BoolProperty negInfForUndefined;
    /// Alternative value for places where RMS is 0
    DoubleProperty undefinedValue;

public:
    typedef double (*ScalarFunc)(const std::array<double, 3>&, const std::array<double, 3>&);
    /// All scalar combinations known
    static const std::unordered_map<std::string, ScalarFunc> ScalarVariants;

protected:
    std::pair<std::string, ScalarFunc> _currentScalar;
};

}  // namespace inviwo
