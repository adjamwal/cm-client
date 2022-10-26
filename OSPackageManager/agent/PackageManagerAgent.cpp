/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PackageManagerAgent.hpp"

#include "PmLogger.hpp"
#include "PmPlatformDependencies.hpp"
#include "PackageManager/IPackageManager.h"

namespace Agent
{

PackageManagerAgent::PackageManagerAgent(const std::string &bootstrapFile,
                                         const std::string &configFile,
                                         IPmPlatformDependencies &dependencies,
                                         IPMLogger &logger)
    : bootstrapFile_ { bootstrapFile },
      configFile_ { configFile },
      logger_ { logger },
      platformDependencies_ { dependencies },
      packageManager_ { nullptr }
{
    SetPMLogger(&logger_);
    InitPackageManager();

    packageManager_ = GetPackageManagerInstance();
    packageManager_->SetPlatformDependencies(&platformDependencies_);
}

PackageManagerAgent::~PackageManagerAgent()
{
    DeinitPackageManager();
    SetPMLogger(nullptr);
}

int32_t PackageManagerAgent::start()
{
    return packageManager_->Start(configFile_.c_str(), bootstrapFile_.c_str());
}

int32_t PackageManagerAgent::stop()
{
    return packageManager_->Stop();
}

bool PackageManagerAgent::configIsValid()
{
    return packageManager_->VerifyPmConfig(configFile_.c_str()) == 0 ? true : false;
}

} // namespace Agent
