/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#include "PackageManager/IPmLogger.h"

class PmLogger : public IPMLogger
{
public:
    PmLogger() {}

    void Log(Severity severity, const char* msgFormatter, ...);
    void Log(Severity severity, const wchar_t* msgFormatter, ...);
    void Log(Severity severity, const char* msgFormatter, va_list args);
    void Log(Severity severity, const wchar_t* msgFormatter, va_list args);

    void SetLogLevel(Severity severity);
};
