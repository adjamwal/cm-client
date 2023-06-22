/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmPlatformDependencies.hpp"
#include "PmCertManager.hpp"
#include "PmCertRetrieverImpl.hpp"

using namespace PackageManager;

PmPlatformDependencies::PmPlatformDependencies()
    : pmConfiguration_ { PmPlatformConfiguration(std::shared_ptr<CMIDAPIProxyAbstract>(new CMIDAPIProxy()),
                                                 std::make_shared<PmCertManager>(std::shared_ptr<IPmCertRetriever>(new PmCertRetrieverImpl))) },
      pmComponentManager_ { PmPlatformComponentManager() }
{ }

IPmPlatformConfiguration &PmPlatformDependencies::Configuration()
{
    return pmConfiguration_;
}

IPmPlatformComponentManager &PmPlatformDependencies::ComponentManager()
{
    return pmComponentManager_;
}
