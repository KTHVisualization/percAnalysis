/*********************************************************************
 *  Author  : Himangshu Saikia
 *  Init    : Wednesday, March 15, 2017 - 15:37:21
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#include <kxtools/volumestitching.h>
#include <inviwo/core/datastructures/volume/volumeram.h>

namespace inviwo
{

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo VolumeStitching::processorInfo_
{
    "org.inviwo.VolumeStitching",      // Class identifier
    "Volume Stitching",                // Display name
    "Volume Operation",              // Category
    CodeState::Experimental,  // Code state
    Tags::None,               // Tags
};

const ProcessorInfo VolumeStitching::getProcessorInfo() const
{
    return processorInfo_;
}


VolumeStitching::VolumeStitching()
    :Processor()
	, volumesIn_("volsIn")
	, volumeOut_("volOut")
	//, dimension_("dims", "Dimension to Stitch")
{
    addPort(volumesIn_);
	addPort(volumeOut_);
    //addProperty(dimension_);
	//dimension_.addOption("x", "X", 0);
	//dimension_.addOption("y", "Y", 1);
	//dimension_.addOption("z", "Z", 2);
}


void VolumeStitching::process()
{
	if (volumesIn_.isConnected()) {
		auto vols = volumesIn_.getData();
		
		if (!vols) {
			return;
		}

		auto numVolumes = vols->GetNumFiles();

		//test for consistency of dimensions of input volumes

		auto vol0 = vols->GetVolume(0);
		auto resDims = vol0->getDimensions();

		for (auto i = 1; i < numVolumes; i++) {

			auto vol = vols->GetVolume(i);
			auto dims = vol->getDimensions();

			if (dims.x != resDims.x || dims.y != resDims.y) {
				LogWarn("Wrong Dimensions in XY!");
				return;
			}
			else {
				resDims.z += dims.z;
			}
		}

		std::shared_ptr<Volume> volume_ = std::make_shared<Volume>(resDims, DataFloat32::get());

		auto drange = vec2(std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest());
		auto vrange = vec2(std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest());
		
		size_t index = 0;
		auto data = static_cast<float*>(volume_->getEditableRepresentation<VolumeRAM>()->getData());

        for (size_t i = 0; i < numVolumes; i++) {

			auto vol = vols->GetVolume(i);
			auto dims = vol->getDimensions();

			const VolumeRAM* vr = vol->getRepresentation< VolumeRAM >();

			drange[0] = std::min(drange[0], static_cast<float>(vol->dataMap_.dataRange[0]));
			drange[1] = std::max(drange[1], static_cast<float>(vol->dataMap_.dataRange[1]));
			vrange[0] = std::min(vrange[0], static_cast<float>(vol->dataMap_.valueRange[0]));
			vrange[1] = std::max(vrange[1], static_cast<float>(vol->dataMap_.valueRange[1]));


			size3_t idx;

			for (idx.z = 0; idx.z < dims.z; idx.z++) {
				for (idx.y = 0; idx.y < dims.y; idx.y++) {
					for (idx.x = 0; idx.x < dims.x; idx.x++) {
						data[index++] = float(vr->getAsDouble(idx));
					}
				}
			}
		}

		volume_->dataMap_.dataRange = drange;
		volume_->dataMap_.valueRange = vrange;

		volumeOut_.setData(volume_);

	}
}

} // namespace

