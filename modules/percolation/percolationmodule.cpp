/*********************************************************************
 *  Author  : Anke Friederici & Tino Weinkauf
 *  Init    : Monday, February 19, 2018 - 11:30:43
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <percolation/percolationmodule.h>
#include <percolation/processors/percolationanalysis.h>
#include <percolation/processors/rawpercolationloader.h>
#include <percolation/processors/scalartransform.h>
#include <percolation/processors/shufflechannel.h>

namespace inviwo {

using namespace discretedata;

PercolationModule::PercolationModule(InviwoApplication* app) : InviwoModule(app, "Percolation") {
    // Add a directory to the search path of the Shadermanager
    // ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Register objects that can be shared with the rest of inviwo here:

    // Processors
    // registerProcessor<PercolationProcessor>();
    registerProcessor<PercolationAnalysis>();
    registerProcessor<RawPercolationLoader>();
    registerProcessor<ScalarTransform>();
    registerProcessor<ShuffleChannel>();
}

}  // namespace inviwo
