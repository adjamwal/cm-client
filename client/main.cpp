/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"
#include "Logger/CMLogger.hpp"
#include "Config.hpp"
#ifdef __APPLE__
#include "crashpad/CrashpadTuner.h"
#endif
#include "cmid/CMIDAPI.h"
#include <iostream>

namespace
{
const std::string logFileName = "csc_cms.log";

void initLogging()
{
    const std::filesystem::path logFilePath = std::filesystem::path(ConfigShared::Config::cmLogPath) / logFileName;
    //initialise Logger before anything else.
    CMLogger::getInstance(logFilePath);
}

#ifdef __APPLE__
bool GetUcIdentity(std::string& identity)
{
    int buflen = 0;
    cmid_result_t result = cmid_get_id(nullptr, &buflen);
    if (result != CMID_RES_INSUFFICIENT_LEN) {
        return false;
    }
    
    std::string cmid(buflen-1, '\0');
    result = cmid_get_id(cmid.data(), &buflen);
    if (result != CMID_RES_SUCCESS) {
        return false;
    }
    
    identity = cmid;
    return true;
}

void initCrashpad()
{
    auto* pCrashpadTuner = CrashpadTuner::getInstance();
    std::string clientId;
    if (GetUcIdentity(clientId))
    {
        pCrashpadTuner->setAgentGuid(clientId);
    }
    else
    {
        CM_LOG_ERROR("Failed to get agent id");
        return;
    }
    
    pCrashpadTuner->setUploadEnabled(true);
    pCrashpadTuner->init(ConfigShared::Config::cmidExePath);
}
#endif
}

int main(int argc, char *argv[])
{
    try {
        initLogging();
#ifdef __APPLE__
        initCrashpad();
#endif
        // TODO:
        //
        // - Signal handlers
        // - atExit() handler
        //
        std::string arguments;
        for ( int i = 0; i < argc; i++ ) {
            arguments += " ";
            arguments += argv[i];
        }
        
        CM_LOG_INFO("CM Command: %s", arguments.c_str() );
        auto service = CloudManagement::Daemon();
        
        // This blocks till we're stopped
        service.start();
    } catch ( const std::exception &rEx ) {
        std::cerr << "Fatal error: " << rEx.what() << std::endl;
        return 1;
    }

    return 0;
}
