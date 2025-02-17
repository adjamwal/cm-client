/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformComponentManager.hpp"
#include "PmPlatformConfiguration.hpp"

#include "PackageManager/IPmPlatformDependencies.h"

class IPmPkgUtil;

class PmPlatformDependencies : public IPmPlatformDependencies
{
public:
    PmPlatformDependencies();
    IPmPlatformConfiguration &Configuration();
    IPmPlatformComponentManager &ComponentManager();

private:
    std::shared_ptr<IPackageUtil> pmPkgUtil_;
    PmPlatformConfiguration pmConfiguration_;
    PmPlatformComponentManager pmComponentManager_;
};
