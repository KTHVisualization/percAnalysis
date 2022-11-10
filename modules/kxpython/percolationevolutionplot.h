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

#pragma once

#include <modules/kxpython/kxpythonmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/properties/dataframeproperty.h>
#include <modules/python3/pythonscript.h>
#include <pybind11/pybind11.h>

namespace inviwo {

/** \docpage{org.inviwo.PercolationEvolutionPlot, Percolation Evolution Plot}
 * ![](org.inviwo.PercolationEvolutionPlot.png?classIdentifier=org.inviwo.PercolationEvolutionPlot)
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __<Inport1>__ <description>.
 *
 * ### Outports
 *   * __<Outport1>__ <description>.
 *
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */

/**
 * \brief VERY_BRIEFLY_DESCRIBE_THE_PROCESSOR
 * DESCRIBE_THE_PROCESSOR_FROM_A_DEVELOPER_PERSPECTIVE
 */
class IVW_MODULE_KXPYTHON_API PercolationEvolutionPlot : public Processor {
public:
    PercolationEvolutionPlot();
    virtual ~PercolationEvolutionPlot() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:

    // Input #1, three columns (H, _, iterations) are choosen for which we plot the heatmap
    DataFrameInport portInPercCurves;
    // Input #2, three columns (pc, delta, iterations) are choosen for which we overlay pc, +- delta at the correct positions
    DataFrameInport portInFitParams;

    CompositeProperty propHeatmapInput;
    // Columns of Data Frame #1 that are used as to assemble the heatmap
    // x (propInputAxis) and y (propOutputAxis)
    DataFrameColumnProperty propInputAxis;
    DataFrameColumnProperty propOutputAxis;
    DataFrameColumnProperty propCurveIterationsAxis;

    CompositeProperty propOverlayInput;

    // Columns of Data Frame #2 that are used for overlaying the heatmap with fitting information
    DataFrameColumnProperty propPcAxis;
    DataFrameColumnProperty propDeltaAxis;
    DataFrameColumnProperty propRMSEAxis;
    DataFrameColumnProperty propParamsIterationsAxis;

    // All properties regarding plotting
    CompositeProperty propPlotting;

    // In order of computation
    BoolProperty propComputationOrder;
    // For axis description: value-based or voxel-based
    BoolProperty propValueBased;
    // Visualize absolut error between fitted and computed function for Erf instead of Value
    BoolProperty propShowError;
    // Enable or disable overlay of Pc
    BoolProperty propOverlayPc;
    // Enable or disable overlay of Delta
    BoolProperty propOverlayDelta;
    // Label 2D Pc
    BoolProperty propShow2dPc;
    // Label 3D Pc
    BoolProperty propShow3dPc;
    // Transpose the entire plot
    BoolProperty propTranspose;
    // Font size in the plot
    IntSizeTProperty propFontSize;
    // Make a separate plot for just the parameters
    BoolProperty propSeparateParams;

    CompositeProperty propIters;
    // Overwrite the iteration parameter (from iterations 1...numIterations) to 
    // something else
    BoolProperty propOverwriteIterParameter;
    // Min and max range for the iteration parameter
    FloatMinMaxProperty propIterMinMax;
    // Floating point or integer value for iter parameter
    BoolProperty propIsIterFloat;
    // New name for the iterations
    StringProperty propIterName;

    // Script with the actual computations
    PythonScriptDisk script;
};

}  // namespace inviwo
