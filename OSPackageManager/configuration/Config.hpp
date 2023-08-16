/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "PackageManager/IPmLogger.h"

#include <string>
#include <filesystem>

namespace PackageManager
{

class Config {
public:
    explicit Config(const std::string& configPath);
    ~Config() = default;
    Config(const Config &other) = delete;
    Config &operator=(const Config &other) = delete;
    Config(Config &&other) = delete;
    Config &operator=(Config &&other) = delete;

    IPMLogger::Severity getLogLevel() const;

    bool reload(const std::string& configPath);
    void onConfigChanged();
    std::function<void()> subscribeForConfigChanges();

    
private:
    bool parsePmConfig();
    static IPMLogger::Severity getDefaultLogLevel();

    std::filesystem::path configPath_;
    IPMLogger::Severity logLevel_ = getDefaultLogLevel();
    
};

} // namespace PackageManager
