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
 * 1. Redistributions of source code must retain the above copyright notice,
 *this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 *FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#pragma once

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/properties/dataframeproperty.h>
#include <modules/kxpython/kxpythonmoduledefine.h>
#include <modules/python3/pythonscript.h>
#include <pybind11/pybind11.h>

namespace inviwo {

/** \docpage{org.inviwo.CurveFitting, Curve Fitting}
 * ![](org.inviwo.CurveFitting.png?classIdentifier=org.inviwo.CurveFitting)
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
class IVW_MODULE_KXPYTHON_API CurveFitting : public Processor {
 public:
  CurveFitting();
  virtual ~CurveFitting() = default;

  virtual void process() override;

  virtual const ProcessorInfo getProcessorInfo() const override;
  static const ProcessorInfo processorInfo_;

 private:
  // Input Data Frame, two columns are choosen for which a curve is fitted
  DataFrameInport portInDataFrame;
  // Output #1 a given number of samples from the fitted function
  DataFrameOutport portOutSamples;
  // Output #2 holds coeffiecients or fitted pc/delta, as well as properties of
  // the fit (e.g. error)
  DataFrameOutport portOutCurveFit;
  // Columns of the Data Frame that are used as to assemble sample point with
  // x (propToFitInputAxis) and y (propToFitOutputAxis)
  DataFrameColumnProperty propToFitInputAxis;
  DataFrameColumnProperty propToFitOutputAxis;

  // Enable for multiple iterations or some other common value
  BoolProperty propMultipleIterations;
  DataFrameColumnProperty propCommonPerIterationAxis;

  // Type of fitting
  OptionPropertyInt propCurveFitType;

  // Degree of the polynomial to be fitted
  IntSizeTProperty propDegree;

  // Number of Samples used for the sampled curve
  IntSizeTProperty propSamples;

  // Script with the actual computations
  PythonScriptDisk script;

  // Estimates for a single iteration
  CompositeProperty propEstimates;
  // Estimation for pc
  FloatProperty propPcEstimate;
  // Estimation for delta
  FloatProperty propDeltaEstimate;
  // Error of the fit
  FloatProperty propError;
};

}  // namespace inviwo
