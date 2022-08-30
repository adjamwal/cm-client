/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <atomic>
#include <thread>

namespace cmid {
    class CCMIDAgentController;
}

namespace ComponentLoader
{

class CMIDLoader {
public:
    CMIDLoader(std::unique_ptr<cmid::CCMIDAgentController> cmid_agent);
    ~CMIDLoader() = default;
    CMIDLoader() = delete;
    CMIDLoader(const CMIDLoader &other) = delete;
    CMIDLoader &operator=(const CMIDLoader &other) = delete;
    CMIDLoader(CMIDLoader &&other) = delete;
    CMIDLoader &operator=(CMIDLoader &&other) = delete;

    void load();
    void unload();

private:
    std::atomic<bool> is_loaded_;
    std::unique_ptr<cmid::CCMIDAgentController> cmid_agent_;
};

} // namespace ComponentLoader
