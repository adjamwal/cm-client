/**
 * @file
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "Daemon.hpp"

int main(int argc, char *argv[])
{

    (void) argc;
    (void) argv;

    auto service = PackageManagerTest::Daemon();

    // This blocks till we're stopped
    service.start();

    return 0;
}
