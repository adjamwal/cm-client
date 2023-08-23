/**
 * @file
 *
 * This needs to be set at the very beginning of the program execution
 * to force log methods that accepts wide characters to work.
 * std::setlocale(LC_CTYPE, "UTF-8");
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */
#pragma once

#include "PackageManager/IPmLogger.h"
#include "IProxyLogger.h"
#include <string>
#include <filesystem>

class PmLogger : public IPMLogger
{
public:
    PmLogger();
    PmLogger(const PmLogger&) = delete;
    PmLogger(PmLogger&&) = delete;
    PmLogger& operator =(const PmLogger&) = delete;
    PmLogger& operator =(PmLogger&&) = delete;
    ~PmLogger() override;
    
    void initFileLogging(const std::string& logFileDir, const std::string& logFileName,
                         const size_t maxFileSize, const size_t maxFiles);

    void Log(Severity severity, const char* msgFormatter, ...) override;
    void Log(Severity severity, const wchar_t* msgFormatter, ...) override;
    void Log(Severity severity, const char* msgFormatter, va_list args) override;
    void Log(Severity severity, const wchar_t* msgFormatter, va_list args) override;

    void SetLogLevel(Severity severity) override;
    
    static PmLogger& getLogger();
    static void initLogger();
    static void releaseLogger();
    
    proxy::IProxyLogger& getProxyLogger();

private:
    class ProxyLogger: public proxy::IProxyLogger
    {
    public:
        explicit ProxyLogger(PmLogger* pLogger);
        ProxyLogger(const ProxyLogger&) = delete;
        ProxyLogger(ProxyLogger&&) = delete;
        ProxyLogger& operator =(const ProxyLogger&) = delete;
        ProxyLogger& operator =(ProxyLogger&&) = delete;
        void Log( int severity, const char* msgFormatter, ... ) override;
        void Log( int severity, const char* msgFormatter, va_list args ) override;
    private:
        PmLogger* pOrigLogger_;
    };
    
    FILE* printDummyFile_;
    Severity curSeverity_ = LOG_DEBUG;
    std::string loggerName_;
    ProxyLogger proxyLogger_;

    void writeLog(Severity severity, const char* msgFormatter, va_list args);
    void writeLog(Severity severity, const wchar_t* msgFormatter, va_list args);
    std::string convertToUtf8(const std::wstring& str);
    void initGlobalLogErrorHandler();
    void initConsoleLogging();
    void initMessagePattern();
    void initDevNullDummyFile();
};

#if defined(__FILE_NAME__)
#  define __FILENAME__ __FILE_NAME__
#else
#  define __FILENAME__ std::filesystem::path(__FILE__).filename().string().c_str()
#endif

#ifndef __FUNCTION__
#   define __FUNCTION__ __func__
#endif

#define PM_LOG_ALERT( fmt, ... ) PmLogger::getLogger().Log( IPMLogger::LOG_ALERT, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define PM_LOG_CRITICAL( fmt, ... ) PmLogger::getLogger().Log( IPMLogger::LOG_CRITICAL, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define PM_LOG_ERROR( fmt, ... ) PmLogger::getLogger().Log( IPMLogger::LOG_ERROR, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define PM_LOG_WARNING( fmt, ... ) PmLogger::getLogger().Log( IPMLogger::LOG_WARNING, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define PM_LOG_NOTICE( fmt, ... ) PmLogger::getLogger().Log( IPMLogger::LOG_NOTICE, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define PM_LOG_INFO( fmt, ... ) PmLogger::getLogger().Log( IPMLogger::LOG_INFO, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define PM_LOG_DEBUG( fmt, ... ) PmLogger::getLogger().Log( IPMLogger::LOG_DEBUG, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
