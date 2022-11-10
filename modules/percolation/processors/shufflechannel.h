/*********************************************************************
 *  Author  : Tino Weinkauf
 *  Init    : Friday, April 05, 2019 - 16:46:52
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
//#include <inviwo/core/ports/volumeport.h>
//#include <inviwo/core/ports/meshport.h>
//#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
//#include <inviwo/core/properties/buttonproperty.h>
//#include <inviwo/core/properties/compositeproperty.h>
//#include <inviwo/core/properties/fileproperty.h>
//#include <inviwo/core/properties/minmaxproperty.h>
//#include <inviwo/core/properties/optionproperty.h>
//#include <inviwo/core/properties/ordinalproperty.h>
//#include <inviwo/core/properties/stringproperty.h>
//#include <inviwo/core/properties/transferfunctionproperty.h>
#include <modules/discretedata/ports/datasetport.h>
#include <modules/discretedata/properties/datachannelproperty.h>

namespace inviwo
{

using namespace discretedata;

/** \docpage{org.inviwo.ShuffleChannel, Shuffle Channel}
    ![](org.inviwo.ShuffleChannel.png?classIdentifier=org.inviwo.ShuffleChannel)

    Explanation of how to use the processor.
    
    ### Inports
      * __<Inport1>__ <description>.
    
    ### Outports
      * __<Outport1>__ <description>.
    
    ### Properties
      * __<Prop1>__ <description>.
      * __<Prop2>__ <description>
*/


/** \class ShuffleChannel
    \brief Shuffles the values in a data channel.
    
    This processor keeps all data values, but re-orders their position in the channel.
    This is of interest for percolation analysis.

    @author Tino Weinkauf
*/
class IVW_MODULE_PERCOLATION_API ShuffleChannel : public Processor
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    ShuffleChannel();
    virtual ~ShuffleChannel() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    ///Our main computation function
    virtual void process() override;

//Ports
public:
    ///Input data
    DataSetInport connInData;

    ///Output data
    DataSetOutport connOutData;

//Properties
public:
    ///Which channel shall be shuffled?
    DataChannelProperty propChannel;

    ///Whether to overwrite the input channel, or add a new one.
    //BoolProperty propGenerateNewChannel;

    ///Randomness properties
    CompositeProperty propGroupRandom;
    ///Whether to use the same seed
    BoolProperty propFixedSeed;
    ///Which seed to use.
    OrdinalProperty<unsigned int> propSeed;


//Attributes
private:

};

} // namespace
