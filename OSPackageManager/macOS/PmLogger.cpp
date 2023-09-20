/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmLogger.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <filesystem>
#include <cerrno>
#include <codecvt>
#include <cwchar>
#include <locale>
#include <memory>

namespace
{
    std::string severityToString(const IPMLogger::Severity s)
    {
        switch(s)
        {
        case IPMLogger::LOG_ALERT:
            return "Alert";
        case IPMLogger::LOG_CRITICAL:
            return "Critical";
        case IPMLogger::LOG_ERROR:
            return "Error";
        case IPMLogger::LOG_WARNING:
            return "Warning";
        case IPMLogger::LOG_NOTICE:
            return "Notice";
        case IPMLogger::LOG_INFO:
            return "Info";
        case IPMLogger::LOG_DEBUG:
        default:
            return "Debug";
        }
        return "Debug";
    }

    static PmLogger* g_pLogger;
    const std::string kConsoleLogName = "console";
}

// utility wrapper to adapt locale-bound facets for wstring/wbuffer convert
template<class Facet>
struct deletable_facet : Facet
{
    using Facet::Facet; // inherit constructors
    ~deletable_facet() {}
};

PmLogger::PmLogger():
        proxyLogger_(this),
        configLogger_(this)
{
    initDevNullDummyFile();
    initConsoleLogging();
    initGlobalLogErrorHandler();
    initMessagePattern();
}

PmLogger::~PmLogger()
{
    auto logger = spdlog::get(loggerName_);
    if (logger)
    {
        logger->flush();
        spdlog::drop(loggerName_);
    }
    
    if (loggerName_ != kConsoleLogName) {
        spdlog::get(kConsoleLogName)->flush();
        spdlog::drop(kConsoleLogName);
    }
    fclose(printDummyFile_);
}

void PmLogger::initFileLogging(const std::string& logFileDir, const std::string& logFileName, const size_t maxFileSize, const size_t maxFiles)
{
    if (loggerName_ != kConsoleLogName)
    {
        auto logger = spdlog::get(loggerName_);
        if (logger)
        {
            logger->flush();
            spdlog::drop(loggerName_);
        }
    }
    
    auto pConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
    bool bDirExists = true;
    if (!std::filesystem::exists(logFileDir))
    {
        std::error_code errCode;
        bDirExists = std::filesystem::create_directories(logFileDir, errCode);
        if (!bDirExists)
        {
            Log(curSeverity_, "Failed to create directory %s, for the log file, error code: %d", logFileDir.c_str(), errCode.value());
        }
    }
    
    if (bDirExists)
    {
        const std::filesystem::path logFilePath = std::filesystem::path(logFileDir) / logFileName;
#ifdef _DEBUG
        auto pStdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt >();
        auto pRotatingSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFilePath.native(), kMaxSize, kMaxFiles);
        std::vector<spdlog::sink_ptr> sinks {pStdoutSink, pRotatingSink};

        auto logger = std::make_shared<spdlog::logger>(logName, sinks.begin(), sinks.end());
        spdlog::register_logger(logger);
#else
        auto logger = spdlog::rotating_logger_mt(logFileName, logFilePath.native(), maxFileSize, maxFiles);
#endif
        
    }
    else
    {
        Log(curSeverity_, "Unable to init logging to file as %s directory does not exist.", logFileDir.c_str());
        //Output to the console if unable to output to the file.
        spdlog::logger logger(logFileName, {pConsoleSink});
    }
    loggerName_ = logFileName;
    initGlobalLogErrorHandler();
    initMessagePattern();
}

void PmLogger::Log(Severity severity, const char* msgFormatter, ...)
{
    if (severity > curSeverity_)
        return;

    va_list va_args;
    va_start(va_args, msgFormatter);
    writeLog(severity, msgFormatter, va_args);

    va_end(va_args);
}

void PmLogger::Log(Severity severity, const wchar_t* msgFormatter, ...)
{
    if (severity > curSeverity_)
        return;

    va_list va_args;
    va_start(va_args, msgFormatter);
    writeLog(severity, msgFormatter, va_args);

    va_end(va_args);
}

void PmLogger::Log(Severity severity, const char* msgFormatter, va_list args)
{
    if (severity > curSeverity_)
        return;
    
    //copy args to prevent destroying original args
    va_list va_args_copy;
    va_copy(va_args_copy, args);
    writeLog(severity, msgFormatter, va_args_copy);
    va_end(va_args_copy);
}

void PmLogger::Log(Severity severity, const wchar_t* msgFormatter, va_list args)
{
    if (severity > curSeverity_)
        return;

    //copy args to prevent destroying original args
    va_list va_args_copy;
    va_copy(va_args_copy, args);
    writeLog(severity, msgFormatter, va_args_copy);
    va_end(va_args_copy);
}

void PmLogger::SetLogLevel(Severity severity)
{
    if (severity < LOG_ALERT || severity > LOG_DEBUG)
    {
        Log(curSeverity_, "Try to set invalid log level: %d", severity);
        return;
    }
    curSeverity_ = severity;
    std::string strLogLevel = severityToString(severity);
    Log(curSeverity_, "Set new log level: %s", strLogLevel.c_str());
}

void PmLogger::writeLog(Severity severity, const char* msgFormatter, va_list args)
{
    std::string strLog = "[";
    strLog += severityToString(severity);
    strLog += "]: ";
    
    va_list copy_args;
    va_copy(copy_args, args);
    int nBufSize = vsnprintf(nullptr, 0, msgFormatter, copy_args);
    va_end(copy_args);
    if (nBufSize <= 0)
    {
        spdlog::get(loggerName_)->info("{}", "Unable to determine buffer size in PmLogger::writeLog");
        spdlog::get(loggerName_)->flush();
        return;
    }
    
    nBufSize += 1;

    std::vector<char> strBuf(static_cast<size_t>(nBufSize), '\0');

    vsnprintf(strBuf.data(), nBufSize, msgFormatter, args);

    strLog += strBuf.data();

    spdlog::get(loggerName_)->info("{}", strLog);
    spdlog::get(loggerName_)->flush();
}

void PmLogger::writeLog(Severity severity, const wchar_t* msgFormatter, va_list args)
{
    std::string strSeverinity = severityToString(severity);
    std::wstring strLog = L"[";
    std::wstring strWSeverinity(strSeverinity.begin(), strSeverinity.end());
    strLog += strWSeverinity;
    strLog += L"]: ";
    
    va_list copy_args;
    va_copy(copy_args, args);
    int nBufSize = vfwprintf(printDummyFile_, msgFormatter, copy_args);
    va_end(copy_args);
    if (nBufSize <= 0)
    {
        spdlog::get(loggerName_)->error("Unable to determine buffer size in PmLogger::writeLog, system error: {}", std::strerror(errno));
        if (ferror (printDummyFile_))
        {
            spdlog::get(loggerName_)->error("{}", "error writing to the temp file.");
        }
        spdlog::get(loggerName_)->flush();
        return;
    }

    nBufSize += 1;
    std::vector<wchar_t> strBuf(nBufSize, L'\0');
    int nRet = vswprintf(strBuf.data(), nBufSize, msgFormatter, args);
    if (nRet < 0)
    {
        spdlog::get(loggerName_)->error("Some error occured during call of the vswprintf in PmLogger::writeLog, system error: {}", std::strerror(errno));
        spdlog::get(loggerName_)->flush();
        return;
    }

    strLog += strBuf.data();
    spdlog::get(loggerName_)->info("{}", convertToUtf8(strLog));
    spdlog::get(loggerName_)->flush();
}

PmLogger& PmLogger::getLogger()
{
    assert(g_pLogger);
    return *g_pLogger;
}

void PmLogger::initLogger()
{
    g_pLogger = new PmLogger();
}

void PmLogger::releaseLogger()
{
    delete g_pLogger;
    g_pLogger = nullptr;
}

std::string PmLogger::convertToUtf8(const std::wstring& str)
{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
#ifdef __clang__
#pragma clang diagnostic pop
#endif

    try
    {
        return converter.to_bytes(str);
    }
    catch(const std::range_error& e)
    {
        spdlog::get(loggerName_)->error("{}: {}", "Wide char string to utf-8 error occured", e.what());
    }
    return "";
}

void PmLogger::initGlobalLogErrorHandler()
{
    spdlog::get(loggerName_)->set_error_handler([](const std::string &msg) { spdlog::get(kConsoleLogName)->error("*** LOGGER ERROR ***: {}", msg); });
}

void PmLogger::initConsoleLogging()
{
    loggerName_ = kConsoleLogName;
    auto console = spdlog::stdout_color_mt(kConsoleLogName);
}

void PmLogger::initMessagePattern()
{
    spdlog::get(loggerName_)->set_pattern("[%b %d %Y %T.%f %z] [thread %t] %v");
}

void PmLogger::initDevNullDummyFile()
{
    printDummyFile_ = fopen("/dev/null", "wb");
}

proxy::IProxyLogger& PmLogger::getProxyLogger()
{
    return proxyLogger_;
}

ConfigShared::IConfigLogger& PmLogger::getConfigLogger()
{
    return configLogger_;
}

PmLogger::ProxyLogger::ProxyLogger(PmLogger* pLogger):
    pOrigLogger_(pLogger)
{
}

PmLogger::ConfigLogger::ConfigLogger(PmLogger* pLogger):
pOrigLogger_(pLogger)
{
}

void PmLogger::ProxyLogger::Log( int severity, const char* msgFormatter, ... )
{
    va_list va_args;
    va_start(va_args, msgFormatter);
    pOrigLogger_->Log(static_cast<PmLogger::Severity>(severity), msgFormatter, va_args);
    va_end(va_args);
}

void PmLogger::ProxyLogger::Log( int severity, const char* msgFormatter, va_list args )
{
    pOrigLogger_->Log(static_cast<PmLogger::Severity>(severity), msgFormatter, args);
}

void PmLogger::ConfigLogger::Log( int severity, const char* msgFormatter, const char *fileName, const char *funcName, long lineNumber, ... ) {
    assert(pOrigLogger_);
    va_list va_args;
    va_start(va_args, lineNumber);
    pOrigLogger_->Log(static_cast<PmLogger::Severity>(severity), msgFormatter, fileName, funcName, lineNumber, va_args);
    va_end(va_args);
}

void PmLogger::ConfigLogger::Log( int severity, const char* msgFormatter, const char *fileName, const char *funcName, long lineNumber, va_list args ) {
    assert(pOrigLogger_);
    pOrigLogger_->Log(static_cast<PmLogger::Severity>(severity), msgFormatter, fileName, funcName, lineNumber, args);
}

void PmLogger::ConfigLogger::SetLogLevel( int severity ) {
    assert(pOrigLogger_);
    pOrigLogger_->SetLogLevel(static_cast<PmLogger::Severity>(severity));
}

const char* PmLogger::ConfigLogger::getKey() const {
    return "pm";
}

