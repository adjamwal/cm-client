/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include "ICMLogger.hpp"

class ICMLogFile;

class CMLogger : public ICMLogger
{
public:

    CMLogger( ICMLogFile& logFile );
    ~CMLogger();
    void logMessage( const CM_LOG_LVL_T severity, const bool bIsStrErr, const char* fileName,
    const char* funcName, long lineNumber, const char* message, ... ) override;
    void setLogLevel( CM_LOG_LVL_T severity ) override;
private:
    CM_LOG_LVL_T logLevel_;
    ICMLogFile& logFile_;
};
