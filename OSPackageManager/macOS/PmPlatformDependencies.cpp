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
#include "PmPkgUtilWrapper.hpp"

using namespace PackageManager;

PmPlatformDependencies::PmPlatformDependencies()
    :
      pmPkgUtil_( std::make_shared<PmPkgUtilWrapper>() ),
      pmConfiguration_ { PmPlatformConfiguration(
        std::make_shared<CMIDAPIProxy>(),
        std::make_shared<PmCertManager>(std::make_shared<PmCertRetrieverImpl>())
      )},
      pmComponentManager_ { PmPlatformComponentManager(
        pmPkgUtil_,
        std::make_shared<CodesignVerifier>(pmPkgUtil_),
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
