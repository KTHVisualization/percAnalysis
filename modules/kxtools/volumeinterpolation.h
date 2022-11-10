/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#include <kxtools/kxtoolsmoduledefine.h>
#include <kxtools/volumesourceseriesdata.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/ordinalproperty.h>


namespace inviwo
{

/** \docpage{org.inviwo.KxVolumeInterpolation, Volume Interpolation}
 * ![](org.inviwo.KxVolumeInterpolation.png?classIdentifier=org.inviwo.KxVolumeInterpolation)
 *
 * Interpolates between two time steps. Uses a series of volumes as input.
 * This is just a quick hack to test getting different results from a upstream processor.
 * 
 * ### Outports
 *   * __Outport__ The interpolated volume.
 *
 * ### Properties
 *   * __Time__ A fractional time value.
 *   * __Time step__ To select the current volume. Hidden.
 *
 */
class IVW_MODULE_KXTOOLS_API KxVolumeInterpolation : public Processor
{
//Friends
//Types
public:    

//Construction / Deconstruction
public:
    KxVolumeInterpolation();
    virtual ~KxVolumeInterpolation() = default;

//Info
public:
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

//Methods
protected:
    ///Our main computation method.
    virtual void process();

//Ports
private:
    ///Volume Series Input
    VolumeSeriesInport portSeries;

    ///Result
    VolumeOutport resVolume;

//Properties
public:
    ///Time.
    DoubleProperty propTime;

//Attributes
private:
};

} // namespace

