/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformComponentManager.hpp"
#include "PmPlatformConfiguration.hpp"

#include "PackageManager/IPmPlatformDependencies.h"
#include "../proxy/CMIDAPIProxy.hpp"

class PmPlatformDependencies : public IPmPlatformDependencies
{
public:
    PmPlatformDependencies();

    IPmPlatformConfiguration &Configuration();
    IPmPlatformComponentManager &ComponentManager();

private:
    PmPlatformConfiguration pmConfiguration_;
    PmPlatformComponentManager pmComponentManager_;
};
