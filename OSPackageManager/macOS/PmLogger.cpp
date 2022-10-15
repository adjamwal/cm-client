/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PmLogger.hpp"

void PmLogger::Log(Severity severity, const char* msgFormatter, ...)
{
    (void) severity;
    (void) msgFormatter;
}

void PmLogger::Log(Severity severity, const wchar_t* msgFormatter, ...)
{
    (void) severity;
    (void) msgFormatter;
}

void PmLogger::Log(Severity severity, const char* msgFormatter, va_list args)
{
    (void) severity;
    (void) msgFormatter;
    (void) args;
}

void PmLogger::Log(Severity severity, const wchar_t* msgFormatter, va_list args)
{
    (void) severity;
    (void) msgFormatter;
    (void) args;
}

void PmLogger::SetLogLevel(Severity severity)
{
    (void) severity;
}
