/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"
#include "PmLogger.hpp"
#include <clocale>
#include <iostream>

int main(int argc, char *argv[])
{
    // TODO:
    //
    // - Signal handlers
    // - atExit() handler
    //

    try {
        std::setlocale(LC_CTYPE, "UTF-8");
        PmLogger::initLogger();
        if(argc < 5)
        {
                //LOG_ERROR("Error: not enough arguments.\n"
                //          "Usage: cmpackagemanager --bootstrap <PathToJSONBoostrapFile> --config-path <PathToJSONConfigFile>");
            return 1;
        }
        
        auto service = PackageManager::Daemon();
        
        for(int i = 0; i < argc;)
        {
            if (std::string("--config-file") == argv[i])
            {
                if(++i < argc)
                {
                    service.setConfigPath(argv[i]);
                }
                ++i;
            }
            else if (std::string("--bootstrap") == argv[i])
            {
                if(++i < argc)
                {
                    service.setBooststrapPath(argv[i]);
                }
                ++i;
            }
            else if (std::string("--log-dir") == argv[i])
            {
                if(++i < argc)
                {
                    service.setLoggerDir(argv[i]);
                }
                ++i;
            }
            else
            {
                ++i;
            }
        }
        
        // This blocks till we're stopped
        service.start();
        
    } catch( std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
    }

    PmLogger::releaseLogger();

    return 0;
}
