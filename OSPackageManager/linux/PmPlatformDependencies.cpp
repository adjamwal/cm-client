/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformDependencies.hpp"
#include "CMIDAPIProxy.hpp"
#include "FileUtilities.hpp"
#ifdef IS_RHEL
#include "PackageUtilRPM.hpp"
#else
#include "PackageUtilDEB.hpp"
#endif

PmPlatformDependencies::PmPlatformDependencies()
        :
#ifdef IS_RHEL
        pmPkgUtil_(std::make_shared<PackageUtilRPM>()),
#else
        pmPkgUtil_(std::make_shared<PackageUtilDEB>()),
#endif
        pmConfiguration_ { PmPlatformConfiguration(std::make_shared<CMIDAPIProxy>()) },
        pmComponentManager_{PmPlatformComponentManager(pmPkgUtil_, std::make_shared<PackageManager::FileUtilities>())}
{}

IPmPlatformConfiguration &PmPlatformDependencies::Configuration()
{
    return pmConfiguration_;
}

IPmPlatformComponentManager &PmPlatformDependencies::ComponentManager()
{
    return pmComponentManager_;
}
