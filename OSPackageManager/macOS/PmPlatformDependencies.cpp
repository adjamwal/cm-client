/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformDependencies.hpp"
#include "PmCertManager.hpp"
#include "PmCertRetrieverImpl.hpp"
#include "PmPkgUtilWrapper.hpp"
#include "FileUtilities.hpp"
#include "PmCodesignVerifier.hpp"

using namespace PackageManager;

PmPlatformDependencies::PmPlatformDependencies()
    : pmConfiguration_ { PmPlatformConfiguration(
        std::make_shared<CMIDAPIProxy>(),
        std::make_shared<PmCertManager>(std::make_shared<PmCertRetrieverImpl>())
      )},
      pmComponentManager_ { PmPlatformComponentManager(
        std::make_shared<PmPkgUtilWrapper>(),
        std::make_shared<CodesignVerifier>(),
        std::make_shared<PackageManager::FileUtilities>()) }
{ }

IPmPlatformConfiguration &PmPlatformDependencies::Configuration()
{
    return pmConfiguration_;
}

IPmPlatformComponentManager &PmPlatformDependencies::ComponentManager()
{
    return pmComponentManager_;
}
