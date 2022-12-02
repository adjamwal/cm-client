/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "ICMLogFile.hpp"
#include <string>
#include <mutex>
#include <filesystem>

class CMLogFile : public ICMLogFile
{
public:
    CMLogFile();
    ~CMLogFile();
    void init( const char* logname = NULL ) override;
    void writeLogLine( const char* logLevel, const char* logLine ) override;
    void setLogConfig( uint32_t fileSize, uint32_t logFiles ) override;
    void deinit();

private:
    std::filesystem::path logFileName_;
    std::mutex mutex_;
    uint32_t maxFileSize_;
    uint32_t maxLogFiles_;
    std::string loggerName_;

    static const uint32_t DEFAULT_MAX_FILE_SIZE = 52428800;
    static const uint32_t DEFAULT_MAX_LOGFILES = 10;
    
    bool isLoggerInitialized();
    void dropLogger();
    void flush();
    bool createLogFile();
};

