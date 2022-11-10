/*********************************************************************
 *  Author  : Himangshu Saikia
 *  Init    : Wednesday, March 15, 2017 - 15:37:21
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <kxtools/kxtoolsmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
//#include <inviwo/core/ports/meshport.h>
//#include <inviwo/core/properties/boolcompositeproperty.h>
//#include <inviwo/core/properties/boolproperty.h>
//#include <inviwo/core/properties/buttonproperty.h>
//#include <inviwo/core/properties/compositeproperty.h>
//#include <inviwo/core/properties/fileproperty.h>
//#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
//#include <inviwo/core/properties/ordinalproperty.h>
//#include <inviwo/core/properties/stringproperty.h>
//#include <inviwo/core/properties/transferfunctionproperty.h>
#include <kxtools/volumesourceseries.h>

namespace inviwo
{

/** \docpage{org.inviwo.VolumeStitching, Volume Stitching}
    ![](org.inviwo.VolumeStitching.png?classIdentifier=org.inviwo.VolumeStitching)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class VolumeStitching
    \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
    
    DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE

    @author Himangshu Saikia
*/
class IVW_MODULE_KXTOOLS_API VolumeStitching : public Processor
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    VolumeStitching();
    virtual ~VolumeStitching() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    ///Our main computation function
    virtual void process() override;

//Ports
public:
	VolumeSeriesInport volumesIn_;
	VolumeOutport volumeOut_;
//Properties
public:
	//TemplateOptionProperty<int> dimension_;
//Attributes
private:

};

} // namespace
