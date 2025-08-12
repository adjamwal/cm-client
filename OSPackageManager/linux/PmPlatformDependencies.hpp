/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformComponentManager.hpp"
#include "PmPlatformConfiguration.hpp"

#include "PackageManager/IPmPlatformDependencies.h"
#include "Gpg/include/GpgUtil.hpp"
#include "OSPackageManager/common/CommandExec.hpp"

class IPmPkgUtil;

class PmPlatformDependencies : public IPmPlatformDependencies
{
public:
    PmPlatformDependencies();
    IPmPlatformConfiguration &Configuration();
    IPmPlatformComponentManager &ComponentManager();

private:
    std::shared_ptr<IGpgUtil>   gpgUtil_;
    std::shared_ptr<ICommandExec> commandExec_;
    PmPlatformConfiguration pmConfiguration_;     // Moved before pmPkgUtil_
    std::shared_ptr<IPackageUtil> pmPkgUtil_;
    PmPlatformComponentManager pmComponentManager_;
};
