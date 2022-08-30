/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "cmid/CMIDAgentController.h"

#include "CMIDLoader.hpp"

#include <memory>

namespace ComponentLoader
{

CMIDLoader::CMIDLoader(std::unique_ptr<cmid::CCMIDAgentController> cmid_agent)
    : cmid_agent_{ std::move(cmid_agent) }
{
}

void CMIDLoader::load()
{
    // Does this block?
    cmid_agent_->Start();

    this->is_loaded_ = true;
}

void CMIDLoader::unload()
{
    this->is_loaded_ = false;
}

} // namespace ComponentLoader
