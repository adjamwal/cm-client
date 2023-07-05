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
#include <string>

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

private:
    FILE* printDummyFile_;
    
    Severity curSeverity_ = LOG_DEBUG;
    std::string loggerName_;

    void writeLog(Severity severity, const char* msgFormatter, va_list args);
    void writeLog(Severity severity, const wchar_t* msgFormatter, va_list args);
    std::string convertToUtf8(const std::wstring& str);
    void initGlobalLogErrorHandler();
    void initConsoleLogging();
    void initMessagePattern();
    void initDevNullDummyFile();
};
