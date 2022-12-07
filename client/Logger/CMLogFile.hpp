/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <string>
#include <mutex>
#include <filesystem>

class CMLogFile
{
public:
    CMLogFile(const char* logname = NULL);
    ~CMLogFile();
    void writeLogLine( const char* logLevel, const char* logLine );
    void setLogConfig( uint32_t fileSize, uint32_t logFiles );

private:
    std::filesystem::path logFileName_;
    std::mutex mutex_;
    uint32_t maxFileSize_;
    uint32_t maxLogFiles_;
    std::string loggerName_;

    static const uint32_t DEFAULT_MAX_FILE_SIZE = 52428800;
    static const uint32_t DEFAULT_MAX_LOGFILES = 10;
    
    void dropLogger();
    void flush();
    bool createLogFile();
};

