#pragma once

#include <kxtools/kxtoolsmoduledefine.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo
{
///Sets the value and data range of the data map.
void IVW_MODULE_KXTOOLS_API SetMinMaxForInviwoDataMap(const char* pData, const size_t NumOfBytes, std::shared_ptr<inviwo::Volume> pVolume);

};
