/**
 * @file
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

int main(int argc, char *argv[])
{
    // TODO:
    //
    // - Signal handlers
    // - atExit() handler
    // - initialize logger
    //
    
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
        else
        {
            ++i;
        }
    }
    

    // This blocks till we're stopped
    service.start();

    return 0;
}
