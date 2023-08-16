/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformComponentManager.hpp"
#include "PmPlatformConfiguration.hpp"

#include "PackageManager/IPmPlatformDependencies.h"
#include "CMIDAPIProxy.hpp"

class IPmPkgUtil;

class PmPlatformDependencies : public IPmPlatformDependencies
{
public:
    PmPlatformDependencies();

    IPmPlatformConfiguration &Configuration();
    IPmPlatformComponentManager &ComponentManager();

private:
    std::shared_ptr<IPmPkgUtil> pmPkgUtil_;
    PmPlatformConfiguration pmConfiguration_;
    PmPlatformComponentManager pmComponentManager_;
};
