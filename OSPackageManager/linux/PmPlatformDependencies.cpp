/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformDependencies.hpp"
#include "CMIDAPIProxy.hpp"
#ifdef IS_RHEL
#include "PackageUtilRPM.hpp"
#endif

PmPlatformDependencies::PmPlatformDependencies()
	    :
	    pmConfiguration_ { PmPlatformConfiguration(std::make_shared<CMIDAPIProxy>()) }
{
	#ifdef IS_RHEL
        pmPkgUtil_ = std::make_shared<PackageUtilRPM>();
        pmComponentManager_ = PmPlatformComponentManager(pmPkgUtil_, std::make_shared<PackageManager::FileUtilities>());
    #endif
}

IPmPlatformConfiguration &PmPlatformDependencies::Configuration()
{
    return pmConfiguration_;
}

IPmPlatformComponentManager &PmPlatformDependencies::ComponentManager()
{
    return pmComponentManager_;
}
