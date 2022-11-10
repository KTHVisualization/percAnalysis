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

#include <modules/kxpython/gaussianrandomfieldgenerator.h>
#include <kxpython/kxpythonmodule.h>
#include <modules/kxtools/performancetimer.h>

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GaussianRandomFieldGenerator::processorInfo_{
    "org.inviwo.GaussianRandomFieldGenerator",      // Class identifier
    "Gaussian Random Field Generator",                // Display name
    "Data Creation",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};
const ProcessorInfo GaussianRandomFieldGenerator::getProcessorInfo() const { return processorInfo_; }

GaussianRandomFieldGenerator::GaussianRandomFieldGenerator()
    : Processor()
    , portOutVolume("outVolume")
    , propDim("dimType", "Dimensions",
        { {"2d", "2D", 0},
         {"3d", "3D", 1} })
    , propSize("volSize", "Size", 128, 16, 1024)
    , propGenerationType("genType", "Type",
        { {"fft", "FFT", 0},
         {"cholesky", "Cholesky", 1} })
    // Minimum 3 since we take the derivative twice
    , propExponent("alpha", "Power Spectrum Exponent", 0.f, 0.f, 15.f)
    , propSigma("sigma", "Kernel height", 1.f, 0.001f, 10.f)
    , propLength("len", "Kernel length", 1.f, 0.001f, 15.f)
    , propSeed("seed", "Seed", 1, 0, 1000)
    , script(InviwoApplication::getPtr()->getModuleByType<KxPythonModule>()->getPath(
        ModulePath::Scripts) + "/grf.py")
{

    addPort(portOutVolume);
    addProperties(propDim, propSize, propGenerationType, propExponent, propSigma, propLength, propSeed);

    propLength.setVisible(false);
    propSigma.setVisible(false);

    script.onChange([&]() { invalidate(InvalidationLevel::InvalidOutput); });

    propGenerationType.onChange([&]() { 
        if (propGenerationType.get() == 0)
        {
            propLength.setVisible(false);
            propSigma.setVisible(false);
            propExponent.setVisible(true);
        }
        else 
        {
            propLength.setVisible(true);
            propSigma.setVisible(true);
            propExponent.setVisible(false);
        }
    });


}

void GaussianRandomFieldGenerator::process() {

    size3_t volumeSize(propSize.get());

    if (propDim.get() == 0)
    {
        volumeSize.z = 1;
    }

    auto volume = std::make_shared<Volume>(volumeSize, DataFloat32::get());
    
    auto locals = pybind11::globals();
    // Expose processor (for any properties) and input/output buffers
    locals["processor"] = pybind11::cast(static_cast<Processor*>(this));
    locals["volume"] = pybind11::cast(volume.get());
    
    PerformanceTimer timer;

    try
    {
        script.run(locals);
    }
    catch (std::exception& e)
    {
        LogError(e.what())
    }

    float timey = timer.ElapsedTime();
    LogInfo("\tGenerated GRF in " << timey << " seconds.");

    portOutVolume.setData(volume);
}

}  // namespace inviwo
