/*********************************************************************
 *  Author  : Anke Friederici & Tino Weinkauf
 *  Init    : Monday, February 19, 2018 - 11:30:43
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

#include <percolation/percolationmoduledefine.h>
#include <inviwo/core/common/inviwomodule.h>

namespace inviwo
{

class IVW_MODULE_PERCOLATION_API PercolationModule : public InviwoModule
{
public:
    PercolationModule(InviwoApplication* app);
    virtual ~PercolationModule() = default;
};

} // namespace
