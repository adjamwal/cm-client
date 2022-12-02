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
    void Init( const char* logname = NULL ) override;
    void WriteLogLine( const char* logLevel, const char* logLine ) override;
    void SetLogConfig( uint32_t fileSize, uint32_t logFiles ) override;
    void Deinit();

private:
    std::filesystem::path m_logFileName;
    std::mutex m_mutex;
    uint32_t m_maxFileSize;
    uint32_t m_maxLogFiles;
    std::string m_loggerName;

    static const uint32_t DEFAULT_MAX_FILE_SIZE = 52428800;
    static const uint32_t DEFAULT_MAX_LOGFILES = 10;
    
    bool IsLoggerInitialized();
    void DropLogger();
    void Flush();
    bool CreateLogFile();
};

