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
        const int nArgsReq = 5; 
        std::setlocale(LC_CTYPE, "UTF-8");
        PmLogger::initLogger();

        std::string arguments;
        for ( int i = 0; i < argc; i++ ) {
            arguments += " ";
            arguments += argv[i];
        }

        if(argc < nArgsReq)
        {
            PM_LOG_ERROR("Error: not enough arguments. Required: %d, provided: %d, command: %s", nArgsReq, argc, arguments.c_str() );
            return 1;
        }

        PM_LOG_INFO("PM Command: %s", arguments.c_str() );
        
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
