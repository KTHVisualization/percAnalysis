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

#include <modules/kxpython/curvefitting.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <kxpython/kxpythonmodule.h>

namespace inviwo
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo CurveFitting::processorInfo_{
    "org.inviwo.CurveFitting",      // Class identifier
    "Curve Fitting",                // Display name
    "Percolation",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo CurveFitting::getProcessorInfo() const { return processorInfo_; }

CurveFitting::CurveFitting()
    : Processor()
    , portInDataFrame("inDataFrame")
    , portOutSamples("outSamples")
    , portOutCurveFit("outCurveFit")
    , propToFitInputAxis("toFitInputAxis", "Input to Fit", portInDataFrame, false, 2)
    , propToFitOutputAxis("toFitOutputAxis", "Output to Fit", portInDataFrame, false, 8)
    , propMultipleIterations("multipleIters", "Multiple Curves to Fit", false)
    , propCommonPerIterationAxis("commonPerIterAxis", "Common Per Iteration", portInDataFrame, false, 1)
    , propCurveFitType("curveFitType", "Curve Fit")
    // Minimum 3 since we take the derivative twice
    , propDegree("degree", "Degree", 3, 3, 15)
    , propSamples("samples", "Samples", 100, 10, 1000)
    , script(InviwoApplication::getPtr()->getModuleByType<KxPythonModule>()->getPath(
        ModulePath::Scripts) + "/curvefitting.py")
    , propEstimates("estimates", "Estimates")
    , propPcEstimate("pcEstimate", "Pc", -1, -1)
    , propDeltaEstimate("deltaEstimate", "Delta", -1, -1)
    , propError("error", "Error", -1, -1)
{
    // Input related ports and properties
    addPort(portInDataFrame);
    addProperties(propToFitInputAxis, propToFitOutputAxis, propMultipleIterations, propCommonPerIterationAxis);

    propCommonPerIterationAxis.setVisible(false);

    propMultipleIterations.onChange([&]()
    {
        if (propMultipleIterations.get())
        {
            propCommonPerIterationAxis.setVisible(true);
        }
        else
        {
            propCommonPerIterationAxis.setVisible(false);
        }
    });

    // Computation related properties
    addProperties(propCurveFitType, propDegree);

    propCurveFitType.addOption("polynomial", "Polynomial", 0);
    propCurveFitType.addOption("erfAdapted", "Adapated Erf (1->0)", 1);
    propCurveFitType.addOption("erf", "Erf (0->1)", 2);

    propCurveFitType.onChange([&]()
    {
        if (propCurveFitType.get() == 0)
        {
            propDegree.setVisible(true);
        }
        else
        {
            propDegree.setVisible(false);
        }
    });

    // Output related port and properties
    addPort(portOutSamples);
    addPort(portOutCurveFit);
    addProperties(propSamples, propEstimates);

    propEstimates.addProperties(propDeltaEstimate, propPcEstimate, propError);

    propPcEstimate.setReadOnly(true);
    propPcEstimate.setSemantics(PropertySemantics::Text);

    propDeltaEstimate.setReadOnly(true);
    propDeltaEstimate.setSemantics(PropertySemantics::Text);

    propError.setReadOnly(true);
    propError.setSemantics(PropertySemantics::Text);

    script.onChange([&]() { invalidate(InvalidationLevel::InvalidOutput); });
}

void CurveFitting::process()
{
    auto dataFrame = portInDataFrame.getData();

    if (!dataFrame) return;

    auto buffertoFitInput = propToFitInputAxis.getBuffer();
    auto bufferToFitOutput = propToFitOutputAxis.getBuffer();

    auto locals = pybind11::globals();
    // Expose processor (for any properties) and input/output buffers
    locals["processor"] = pybind11::cast(static_cast<Processor*>(this));
    locals["toFitInputBuffer"] = pybind11::cast(static_cast<const BufferBase *>(buffertoFitInput.get()));
    locals["toFitOutputBuffer"] = pybind11::cast(static_cast<const BufferBase *>(bufferToFitOutput.get()));

    if (propMultipleIterations.get())
    {
        auto bufferCommonPerIter = propCommonPerIterationAxis.getBuffer();
        locals["commonPerIterBuffer"] = pybind11::cast(static_cast<const BufferBase *>(bufferCommonPerIter.get()));
    }

    auto outDataFrameCurveFit = std::make_shared<DataFrame>();
    std::vector<std::shared_ptr<TemplateColumn<float>>> paramColumns; 
    
    if (propCurveFitType.get() == 0)
    {
        for (int paramId = propDegree.get(); paramId >= 0; paramId--)
        {
            std::string columnName = "Coefficient for x^" + std::to_string(paramId);
            std::string identifierName = "coeffBuffer_" + std::to_string(paramId);
            paramColumns.emplace_back(outDataFrameCurveFit->addColumn<float>(columnName));
            locals[identifierName.data()] = pybind11::cast(static_cast<BufferBase *>(paramColumns.back()->getTypedBuffer().get()));
        }
        paramColumns.emplace_back(outDataFrameCurveFit->addColumn<float>("Pc Estimate"));
        locals["pcEstimateBuffer"] = pybind11::cast(static_cast<BufferBase *>(paramColumns.back()->getTypedBuffer().get()));
        paramColumns.emplace_back(outDataFrameCurveFit->addColumn<float>("Delta Estimate"));
        locals["deltaEstimateBuffer"] = pybind11::cast(static_cast<BufferBase *>(paramColumns.back()->getTypedBuffer().get()));
    }
    else
    {
        paramColumns.emplace_back(outDataFrameCurveFit->addColumn<float>("Pc"));
        locals["pcBuffer"] = pybind11::cast(static_cast<BufferBase *>(paramColumns.back()->getTypedBuffer().get()));
        paramColumns.emplace_back(outDataFrameCurveFit->addColumn<float>("Delta"));
        locals["deltaBuffer"] = pybind11::cast(static_cast<BufferBase *>(paramColumns.back()->getTypedBuffer().get()));
    }

    paramColumns.emplace_back(outDataFrameCurveFit->addColumn<float>("RMSE"));
    locals["rmseBuffer"] = pybind11::cast(static_cast<BufferBase *>(paramColumns.back()->getTypedBuffer().get()));

    if (propMultipleIterations.get())
    {
        paramColumns.emplace_back(outDataFrameCurveFit->addColumn<float>(propCommonPerIterationAxis.getColumn()->getHeader()));
        locals["commonPerIterParamsBuffer"] = pybind11::cast(static_cast<BufferBase *>(paramColumns.back()->getTypedBuffer().get()));
    }

    // Sample dataframe

    auto outDataFrameSamples = std::make_shared<DataFrame>();
    auto xSamples = outDataFrameSamples->addColumn<float>("X", propSamples.get());
    locals["xSamplesBuffer"] = pybind11::cast(static_cast<BufferBase *>(xSamples->getTypedBuffer().get()));
    auto ySamples = outDataFrameSamples->addColumn<float>("Y", propSamples.get());
    locals["ySamplesBuffer"] = pybind11::cast(static_cast<BufferBase *>(ySamples->getTypedBuffer().get()));

    if (propMultipleIterations.get())
    {
        auto commonPerIterSamples = outDataFrameSamples->addColumn<float>(propCommonPerIterationAxis.getColumn()->getHeader());
        locals["commonPerIterSamplesBuffer"] = pybind11::cast(static_cast<BufferBase *>(commonPerIterSamples->getTypedBuffer().get()));
    }

    try
    {
        script.run(locals);
    }
    catch (std::exception& e)
    {
        LogError(e.what())
    }

    portOutSamples.setData(outDataFrameSamples);
    portOutCurveFit.setData(outDataFrameCurveFit);
}

}  // namespace inviwo
