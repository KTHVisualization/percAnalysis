/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <modules/kxpython/percolationplot.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <kxpython/kxpythonmodule.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PercolationPlot::processorInfo_{
    "org.inviwo.PercolationPlot",      // Class identifier
    "Percolation Plot",                // Display name
    "Undefined",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo PercolationPlot::getProcessorInfo() const { return processorInfo_; }

PercolationPlot::PercolationPlot()
    : Processor()
    , portInPercCurves("inPercCurves")
    , portInFitParams("inFitParams")
    , propInputType("inputType", "Input Type", 
        {{"single", "Single Curve", 0},
         {"multiple", "Multiple Iterations", 1},
         {"shuffled", "Non-Shuffled vs. Shuffled", 2} })
    , propIteration("iteration", "Iteration", 1, 1)
    , propCurveInput("curve", "Curve Input")
    , propInputAxis("inputAxis", "H", portInPercCurves, false, 2)
    , propOutputAxis("outputAxis", "Value", portInPercCurves, false, 8)
    , propCurveIterationsAxis("curveIterationsAxis", "Common Per Iteration", portInPercCurves, true, 1)
    , propOverlayInput("overlay", "Overlay Input")
    , propPcAxis("pcAxis", "Pc", portInFitParams, false, 1)
    , propDeltaAxis("deltaAxis", "Delta", portInFitParams, false, 2)
    , propParamsIterationsAxis("paramsIterationsAxis", "Common Per Iteration", portInFitParams, false, 4)
    , propPlotting("plot", "Plotting")
    , propComputationOrder("computationOrder", "In Computation Order", false)
    , propValueBased("valueBased", "Value-Based Samples", false)
    , propShowError("showError", "Show Error", false)
    , propShowFitted("showFitted", "Show Fitted", false)
    , propOverlayPc("enableOverlayPc", "Enable Overlay of Pc", true)
    , propOverlayDelta("enableOverlayDelta", "Enable Overlay of Delta", false)
    , propShow2dPc("show2dPc", "Show 2D Pc", false)
    , propShow3dPc("show3dPc", "Show 3D Pc", false)
    , propFontSize("fontSize", "Font Size", 10, 6, 50)
    , script(InviwoApplication::getPtr()->getModuleByType<KxPythonModule>()->getPath(
        ModulePath::Scripts) + "/percplot.py") {

    // addPort(portOutImage);
    addPort(portInPercCurves);
    // May be optional? (-> Disable overlay)
    addPort(portInFitParams);

    addProperties(propInputType, propIteration, propCurveInput, propOverlayInput, propPlotting);

    propCurveInput.addProperties(propInputAxis, propOutputAxis, propCurveIterationsAxis);
    propOverlayInput.addProperties(propPcAxis, propDeltaAxis, propParamsIterationsAxis);
    propPlotting.addProperties(propComputationOrder, propValueBased, propShowFitted,
                               propShowError, propOverlayPc, propOverlayDelta, 
        propShow2dPc, propShow3dPc, propFontSize);

    script.onChange([&]() { invalidate(InvalidationLevel::InvalidOutput); });
}

void PercolationPlot::process() {

    auto locals = pybind11::globals();
    // Expose processor (for any properties) and input/output buffers
    locals["processor"] = pybind11::cast(static_cast<Processor*>(this));
    locals["hValues"] = pybind11::cast(static_cast<const BufferBase *>(propInputAxis.getBuffer().get()));
    locals["outValues"] = pybind11::cast(static_cast<const BufferBase *>(propOutputAxis.getBuffer().get()));

    // Setting for multiple iterations
    if (propInputType.get() == 1)
    {
        locals["curveIters"] = pybind11::cast(static_cast<const BufferBase *>(propCurveIterationsAxis.getBuffer().get()));
        locals["paramIters"] = pybind11::cast(static_cast<const BufferBase *>(propParamsIterationsAxis.getBuffer().get()));
    }

    locals["pcValues"] = pybind11::cast(static_cast<const BufferBase *>(propPcAxis.getBuffer().get()));
    locals["deltaValues"] = pybind11::cast(static_cast<const BufferBase *>(propDeltaAxis.getBuffer().get()));


    try
    {
        script.run(locals);
    }
    catch (std::exception& e)
    {
        LogError(e.what())
    }
}

}  // namespace inviwo
