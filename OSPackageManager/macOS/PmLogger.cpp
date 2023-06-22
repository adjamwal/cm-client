/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmLogger.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <iostream>

namespace
{
    std::string severityToString(const IPMLogger::Severity s)
    {
        switch(s)
        {
        case IPMLogger::LOG_ALERT:
            return "Alert";
        case IPMLogger::LOG_CRITICAL:
            return "Cretical";
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
            return "Debug:";
        }
        return "Debug";
    }

    static PmLogger g_logger;
}


PmLogger::PmLogger()
{
    spdlog::stdout_color_mt("console");
}

void PmLogger::Log(Severity severity, const char* msgFormatter, ...)
{
    if (severity > m_curSeverity)
        return;

    va_list va_args;
    va_start(va_args, msgFormatter);
    writeLog(severity, msgFormatter, va_args);

    va_end(va_args);
}

void PmLogger::Log(Severity severity, const wchar_t* msgFormatter, ...)
{
    if (severity > m_curSeverity)
        return;

    va_list va_args;
    va_start(va_args, msgFormatter);
    writeLog(severity, msgFormatter, va_args);

    va_end(va_args);
}

void PmLogger::Log(Severity severity, const char* msgFormatter, va_list args)
{
    if (severity > m_curSeverity)
        return;

    writeLog(severity, msgFormatter, args);
}

void PmLogger::Log(Severity severity, const wchar_t* msgFormatter, va_list args)
{
    if (severity > m_curSeverity)
        return;

    writeLog(severity, msgFormatter, args);
}

void PmLogger::SetLogLevel(Severity severity)
{
    m_curSeverity = severity;
}

void PmLogger::writeLog(Severity severity, const char* msgFormatter, va_list args)
{
    std::string strLog = severityToString(severity);
    strLog += ": ";
    
    va_list copy_args;
    va_copy(copy_args, args);
    int nBufSize = vsnprintf(nullptr, 0, msgFormatter, copy_args);
    va_end(copy_args);
    if (nBufSize <= 0)
        return;
    
    nBufSize += 1;

    std::vector<char> strBuf(static_cast<size_t>(nBufSize), '\0');

    vsnprintf(strBuf.data(), nBufSize, msgFormatter, args);

    strLog += strBuf.data();

    spdlog::get("console")->info("{}", strLog);
}

void PmLogger::writeLog(Severity severity, const wchar_t* msgFormatter, va_list args)
{
    std::string strSeverinity = severityToString(severity);
    std::wstring strLog(strSeverinity.begin(), strSeverinity.end());
    strLog += L": ";
    size_t nBufSize = 2048;

    std::vector<wchar_t> strBuf;
    int nRet = -1;
    int nIter = 0;

    do
    {
        strBuf.resize(0);
        strBuf.resize(nBufSize + 1, L'\0');
        va_list copy_args;
        va_copy(copy_args, args);
        nRet = vswprintf(strBuf.data(), nBufSize, msgFormatter, copy_args);
        va_end(copy_args);
        nBufSize *= 2;
        ++nIter;
    } while (nRet < 0 && nIter < 4);
    strLog += strBuf.data();

    //spdlog::get("console")->info(L"{}", strLog);
    std::wcout << strLog << std::endl;
}

IPMLogger& PmLogger::GetCurrentLogger()
{
    return g_logger;
}
