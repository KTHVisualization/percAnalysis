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

#include <modules/kxpython/percolationevolutionplot.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <kxpython/kxpythonmodule.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo PercolationEvolutionPlot::processorInfo_{
    "org.inviwo.PercolationEvolutionPlot",      // Class identifier
    "Percolation Evolution Plot",                // Display name
    "Percolation",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo PercolationEvolutionPlot::getProcessorInfo() const { return processorInfo_; }

PercolationEvolutionPlot::PercolationEvolutionPlot()
    : Processor()
    , portInPercCurves("inPercCurves")
    , portInFitParams("inFitParams")
    , propHeatmapInput("heatmap", "Heatmap Input")
    , propInputAxis("inputAxis", "H", portInPercCurves, false, 2)
    , propOutputAxis("outputAxis", "Value", portInPercCurves, false, 8)
    , propCurveIterationsAxis("curveIterationsAxis", "Common Per Iteration", portInPercCurves, false, 1)
    , propOverlayInput("overlay", "Overlay Input" )
    , propPcAxis("pcAxis", "Pc", portInFitParams, false, 1)
    , propDeltaAxis("deltaAxis", "Delta", portInFitParams, false, 2)
    , propRMSEAxis("rmseAxis", "RMSE", portInFitParams, false, 3)
    , propParamsIterationsAxis("paramsIterationsAxis", "Common Per Iteration", portInFitParams, false, 4)
    , propPlotting("plot", "Plotting")
    , propComputationOrder("computerationOrder", "In Computation Order", false)
    , propValueBased("valueBased", "Value-Based Samples", false)
    , propShowError("showError", "Show Error Instead of Value", false)
    , propOverlayPc("enableOverlayPc", "Enable Overlay of Pc", true)
    , propOverlayDelta("enableOverlayDelta", "Enable Overlay of Delta", false)
    , propShow2dPc("show2dPc", "Show 2D Pc", false)
    , propShow3dPc("show3dPc", "Show 3D Pc", false)
    , propTranspose("transpose", "Transpose Plot", false)
    , propFontSize("fontSize", "Font Size", 10, 6, 50)
    , propSeparateParams("separateParams", "Parameter Only Plot", false)
    , propIters("iters", "Iteration Parameter")
    , propOverwriteIterParameter("overwrite", "Overwrite", false)
    , propIterMinMax("minMax", "Range")
    , propIsIterFloat("isFloat", "Is of Type Float")
    , propIterName("name", "Name", "")
    , script(InviwoApplication::getPtr()->getModuleByType<KxPythonModule>()->getPath(
        ModulePath::Scripts) + "/percheatmap.py") {

    // addPort(portOutImage);
    addPort(portInPercCurves);
    // May be optional? (-> Disable overlay)
    addPort(portInFitParams);

    addProperties(propHeatmapInput, propOverlayInput, propPlotting);

    propHeatmapInput.addProperties(propInputAxis, propOutputAxis, propCurveIterationsAxis);
    propOverlayInput.addProperties(propPcAxis, propDeltaAxis, propRMSEAxis, propParamsIterationsAxis);
    propPlotting.addProperties(propComputationOrder, propValueBased, propShowError, propOverlayPc,
                               propOverlayDelta, propShow2dPc, propShow3dPc, 
        propTranspose, propFontSize, propSeparateParams, propIters);
    propIters.addProperties(propOverwriteIterParameter, propIsIterFloat, propIterMinMax, propIterName);

    script.onChange([&]() { invalidate(InvalidationLevel::InvalidOutput); });
}

void PercolationEvolutionPlot::process() {

    auto locals = pybind11::globals();
    // Expose processor (for any properties) and input/output buffers
    locals["processor"] = pybind11::cast(static_cast<Processor*>(this));
    locals["hValues"] = pybind11::cast(static_cast<const BufferBase *>(propInputAxis.getBuffer().get()));
    locals["outValues"] = pybind11::cast(static_cast<const BufferBase *>(propOutputAxis.getBuffer().get()));
    locals["curveIters"] = pybind11::cast(static_cast<const BufferBase *>(propCurveIterationsAxis.getBuffer().get()));
    
    locals["pcValues"] = pybind11::cast(static_cast<const BufferBase *>(propPcAxis.getBuffer().get()));
    locals["deltaValues"] = pybind11::cast(static_cast<const BufferBase *>(propDeltaAxis.getBuffer().get()));
    locals["rmseValues"] = pybind11::cast(static_cast<const BufferBase *>(propRMSEAxis.getBuffer().get()));
    locals["paramIters"] = pybind11::cast(static_cast<const BufferBase *>(propParamsIterationsAxis.getBuffer().get()));
 
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
