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
    std::error_code ecode;
    PM_LOG_INFO( "PM Agent - Using config path: %s, exists: %s, code: %d, msg: %s", configFile_.c_str(),
                 std::filesystem::exists(configFile_, ecode) ? "Yes" : "No", ecode.value(), ecode.message().c_str() );

    PM_LOG_INFO( "PM Agent - Using bootstrap path: %s, exists: %s, code: %d, msg: %s", bootstrapFile_.c_str(),
                 std::filesystem::exists(bootstrapFile_, ecode) ? "Yes" : "No", ecode.value(), ecode.message().c_str() );

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
