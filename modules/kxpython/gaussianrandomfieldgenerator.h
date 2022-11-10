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
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/ports/volumeport.h>
#include <modules/python3/pythonscript.h>
#include <pybind11/pybind11.h>

namespace inviwo {

/** \docpage{org.inviwo.GaussianRandomFieldGenerator, Gaussian Random Field Generator}
 * ![](org.inviwo.GaussianRandomFieldGenerator.png?classIdentifier=org.inviwo.GaussianRandomFieldGenerator)
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
class IVW_MODULE_KXPYTHON_API GaussianRandomFieldGenerator : public Processor {
public:
    GaussianRandomFieldGenerator();
    virtual ~GaussianRandomFieldGenerator() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    // Volume result
    VolumeOutport portOutVolume;

    // 2D (2Dx1) or 3D volume
    OptionPropertyInt propDim;

    // Size of the resulting volume
    IntSizeTProperty propSize;

    // Type of generation (Cholesky, FFT)
    OptionPropertyInt propGenerationType;
    // Power spectrum exponent for FFT
    FloatProperty propExponent;
    // Sigma parameter in Covariance for Cholesky
    FloatProperty propSigma;
    // Length parameter in Covariance for Cholesky
    FloatProperty propLength;

    // Properties for randomness
    IntProperty propSeed;

    // Script with the actual computations
    PythonScriptDisk script;

};

}  // namespace inviwo
