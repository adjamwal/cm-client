/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformDependencies.hpp"
#include "PmCertManager.hpp"
#include "PmCertRetrieverImpl.hpp"
#include "CMIDAPIProxy.hpp"
#include "FileUtilities.hpp"
#ifdef IS_RHEL
#include "PackageUtilRPM.hpp"
#else
#include "PackageUtilDEB.hpp"
#endif

PmPlatformDependencies::PmPlatformDependencies()
        :
        gpgUtil_(std::make_shared<GpgUtil>()),
        commandExec_(std::make_shared<CommandExec>()),
#ifdef IS_RHEL
        pmPkgUtil_(std::make_shared<PackageUtilRPM>(*std::move(commandExec_), *std::move(gpgUtil_))),
#else
        pmPkgUtil_(std::make_shared<PackageUtilDEB>()),
#endif
        pmConfiguration_ { PmPlatformConfiguration(
                std::make_shared<CMIDAPIProxy>(), 
                std::make_shared<PackageManager::PmCertManager>(std::make_shared<PackageManager::PmCertRetrieverImpl>())
                )},
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
