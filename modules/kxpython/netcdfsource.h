/*********************************************************************
 *  Author  : Wiebke Koepp
 *  Init    : Monday, September 24, 2018 - 15:00:28
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <kxpython/kxpythonmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <modules/python3/pythonscript.h>
#include <pybind11/pybind11.h>

namespace inviwo
{
namespace kth
{

/** \docpage{org.inviwo.NetCDFSource, NetCDF Source}
    ![](org.inviwo.NetCDFSource.png?classIdentifier=org.inviwo.NetCDFSource)

    The processor reads a given variable with dimensions [time, latitude, longtitude] from all .nc files
    in a given folder. We assume the files are given in temporal order. For the variable, we assume a single level,
    latitude given in degrees north and longtitude in degrees east. 
    
    ### Outports
      * __outVolume__ Output volume with dimensions [longtitude, latitude, time].
    
    ### Properties
      * __folder__ Folder with .nc files.
      * __variable__ Variable to read from each file.
      * __flipLatitude__ Flips the latitude axis. This should be done if latitude is given ranging from 90N to -90N
        such that the flipped version ranges from -90N to 90N.
      * __rollLongtitude__ Rolls the longtitude axis. This should be done if longtitude is given ranging from 0E to 360E 
        such that the rolled version ranged from -180E to 180N with 0 in the center.
*/


/** \class NetCDFSource
    \brief Reads and stacks a given variable from NetCDF files within a given folder. 

    @author Wiebke Koepp
*/
class IVW_MODULE_KXPYTHON_API NetCDFSource : public Processor
{ 
//Friends
//Types
public:

//Construction / Deconstruction
public:
    NetCDFSource();
    virtual ~NetCDFSource() = default;

//Methods
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void process() override;
//Ports
public:
    ///Result volume filled with stacked .nc files
    VolumeOutport outVolume;

//Properties
public:
    ///The directory to read from
    DirectoryProperty propFolder;

    ///Variable to choose from NetCDF
    StringProperty propVariable;

    ///Flip Latitude so that is ranges from -90N to 90N: 
    //Values can be saved ranging from [90, -90], i.e for a timeslice [latitude, longtitude]
    //the first value would be the one for 90 degree north, 0 degrees east, whereas we would want the value
    //for -90 degreest north, 0 degrees east.
    BoolProperty propFlipLatitude;

    ///Roll Longtitude so that is ranges from -180E to 180E
    //Longtitude is often visualized with 0 in the center, i.e. ranging from -180, 180, but the data might be 
    //arranged as ranging from 0 to 360
    //We do not include a property for flipping 
    BoolProperty propRollLongtitude;

    ///Button for showing all variables with dimensions [time, latitude, longtitude]
    ButtonProperty propPrintVariables;

    ///Checkbox for printing bounding box and volume size
    ButtonProperty propPrintVolumeInfo;

//Attributes
private:
    // The script that actually reads from file and fills the volume
    PythonScriptDisk script;

};

} // namespace
} // namespace
