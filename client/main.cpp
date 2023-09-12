/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"
#include "Logger/CMLogger.hpp"
#include "Config.hpp"
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

}

int main(int argc, char *argv[])
{
    try {
        initLogging();
        // TODO:
        //
        // - Signal handlers
        // - atExit() handler
        //
        (void) argc;
        (void) argv;
        auto service = CloudManagement::Daemon();

        // This blocks till we're stopped
        service.start();
    }
    catch ( const CMLogger::logger_exception &rLogEx ) {
        //Do not log on logger exception
        std::cerr << "Fatal error: " << rLogEx.what() << std::endl;
        return 1;
    }
    catch ( const std::exception &rEx )
    {
        CM_LOG_ERROR("Fatal error [%s]", rEx.what());
        return 2;
    }

    return 0;
}
