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
    (void) argc;
    (void) argv;

    auto service = std::make_unique<PackageManager::Daemon>();

    // This blocks till we're stopped
    service->start();

    return 0;
}
