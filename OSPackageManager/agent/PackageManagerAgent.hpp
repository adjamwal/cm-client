/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

class IPackageManager;
class IPmPlatformDependencies;
class IPMLogger;

#include <string>

namespace Agent
{

class PackageManagerAgent {
public:
    PackageManagerAgent(const std::string &bootstrapFile,
                        const std::string &configFile,
                        IPmPlatformDependencies &dependencies,
                        IPMLogger &logger);
    ~PackageManagerAgent();

    int32_t start();
    int32_t stop();

    bool configIsValid();

private:
    std::string bootstrapFile_;
    std::string configFile_;

    IPMLogger &logger_;
    IPmPlatformDependencies &platformDependencies_;
    IPackageManager *packageManager_;
};


} // namespace Agent
