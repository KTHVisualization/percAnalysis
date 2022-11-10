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

#include <kxtools/volumeinterpolation.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

namespace inviwo
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo KxVolumeInterpolation::processorInfo_
{
    "org.inviwo.KxVolumeInterpolation", // Class identifier
    "Volume Interpolation",             // Display name
    "Data Input",                       // Category
    CodeState::Experimental,            // Code state
    Tags::CPU,                          // Tags
};

const ProcessorInfo KxVolumeInterpolation::getProcessorInfo() const
{
    return processorInfo_;
}


KxVolumeInterpolation::KxVolumeInterpolation()
    :Processor()
    ,portSeries("volumeSeries")
    ,resVolume("volumeOutport")
    ,propTime("time", "Time", 0.5f, 0, 1, 0.1f)
{
    addPort(portSeries);
    addPort(resVolume);

    addProperty(propTime);

    //Take care of the connection to a file series
    portSeries.onChange([&]()
    {
        auto pSeries = portSeries.getData();
        if (pSeries)
        {
            //Scale the physical time according to the number of files
            const double fMin = 0;
            const double fMax = (double)(pSeries->GetNumFiles() - 1);
            if (propTime.getMinValue() != fMin) propTime.setMinValue(fMin);
            if (propTime.getMaxValue() != fMax) propTime.setMaxValue(fMax);
        }

    });
}


void KxVolumeInterpolation::process()
{
    //inviwo::InvalidationLevel ILevel = this->getInvalidationLevel();
    //LogInfo("InvalidationLevel " << (int)ILevel);

    if (!portSeries.isConnected()) return;

    //Get Input
    auto pSeries = portSeries.getData();
    if (!pSeries) return;
    const size_t NumFiles = pSeries->GetNumFiles();

    //Get param
    const double PhysicalTime = propTime.get();
    const int LeftIdx = int(PhysicalTime);
    const int RightIdx = (LeftIdx < NumFiles-1) ? LeftIdx + 1 : LeftIdx;
    const double t = PhysicalTime - double(LeftIdx);
    const double tOther = 1.0 - t;

    //LogInfo("Setting Left Idx to " << LeftIdx << " and Right Idx to " << RightIdx << " with t = " << t);

    //Get input data
    std::shared_ptr< Volume > pLeftVol = pSeries->GetVolume(LeftIdx);
    std::shared_ptr< Volume > pRightVol = pSeries->GetVolume(RightIdx);
    if (!pLeftVol || !pRightVol) return;

    //Get Representation
    const VolumeRAM* pLeft = pLeftVol->getRepresentation< VolumeRAM >();
    const VolumeRAM* pRight = pRightVol->getRepresentation< VolumeRAM >();
    if (!pLeft || !pRight) return;

    //Same Dimensions?
    if (pLeft->getDimensions() != pRight->getDimensions()) return;

    //Prepare output memory
    //auto OutVolume = std::make_shared<Volume>(pLeft->getDimensions(), pLeft->getDataFormat());
    //OutVolume->dataMap_ = pLeftVol->dataMap_;
    //VolumeRAM* pOut = OutVolume->getEditableRepresentation< VolumeRAM >();
    Volume* OutVolume = pLeftVol->clone();
    VolumeRAM* pOut = OutVolume->getEditableRepresentation< VolumeRAM >();

    //Interpolate Data
    const size3_t Dims = pOut->getDimensions();
    size3_t Index;
    for (Index.z = 0; Index.z < Dims.z; Index.z++)
    {
        for (Index.y = 0; Index.y < Dims.y; Index.y++)
        {
            for (Index.x = 0; Index.x < Dims.x; Index.x++)
            {
				const double L = pLeft->getAsDouble(Index);   
                const double R = pRight->getAsDouble(Index);

                pOut->setFromDouble(Index, tOther * L + t * R);
            }
        }
    }

    //Push!
    resVolume.setData(OutVolume);
}

} // namespace
