/*********************************************************************
 *  Author  : Wiebke Koepp
 *  Init    : Monday, September 24, 2018 - 15:00:28
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <kxpython/netcdfsource.h>
#include <kxpython/kxpythonmodule.h>

namespace inviwo
{
namespace kth
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo NetCDFSource::processorInfo_
{
    "org.inviwo.NetCDFSource",      // Class identifier
    "NetCDF Source",                // Display name
    "Data Input",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo NetCDFSource::getProcessorInfo() const
{
    return processorInfo_;
}


NetCDFSource::NetCDFSource()
    : Processor()
    , outVolume("outVolume")
    , propFolder("folder", "Folder")
    , propVariable("variable", "Variable")
    , propFlipLatitude("flipLatitude", "Flip Latitude", true)
    , propRollLongtitude("rollLongtitude", "Roll Longtitude", false)
    , propPrintVariables("printVariables", "Print Variables")
    , propPrintVolumeInfo("printVolumeInfo", "Volume Info")
    , script(InviwoApplication::getPtr()->getModuleByType<KxPythonModule>()->getPath(
        ModulePath::Scripts) + "/readNetCDFFromFolder.py")
{
    // Ports
    addPort(outVolume);
    
    // Properties
    addProperty(propFolder);
    addProperty(propVariable);
    addProperty(propFlipLatitude);
    addProperty(propRollLongtitude);

    addProperty(propPrintVariables);
    addProperty(propPrintVolumeInfo);

    // onChange functions are set in the Python script
    auto runscript = [this]()
    {
        auto locals = pybind11::globals();
        // This processor is not registered to the Python module, thus we cast to a regular processor
        locals["self"] = pybind11::cast(static_cast<Processor*>(this));
        try
        {
            script.run(locals);
        }
        catch (std::exception& e)
        {
            LogError(e.what())
        }
        invalidate(InvalidationLevel::InvalidOutput);
    };

    script.onChange([runscript]() { runscript(); });

    runscript();
}

void NetCDFSource::process()
{
    // Do nothing, everything is done in the script through onChange bindings
}


} // namespace
} // namespace

