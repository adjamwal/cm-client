/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */
#include "CMLogger.hpp"
#include "ICMLogFile.hpp"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

const char* LogLevelStr( const CM_LOG_LVL_T level )
{
    const char* levelStr = "";
    switch( level ) {
    case CM_LOG_LVL_T::CM_LOG_ALERT:
        levelStr = "ALERT";
        break;
    case CM_LOG_LVL_T::CM_LOG_CRITICAL:
        levelStr = "CRITICAL";
        break;
    case CM_LOG_LVL_T::CM_LOG_ERROR:
        levelStr = "ERROR";
        break;
    case CM_LOG_LVL_T::CM_LOG_WARNING:
        levelStr = "WARNING";
        break;
    case CM_LOG_LVL_T::CM_LOG_NOTICE:
        levelStr = "NOTICE";
        break;
    case CM_LOG_LVL_T::CM_LOG_INFO:
        levelStr = "INFO";
        break;
    case CM_LOG_LVL_T::CM_LOG_DEBUG:
        levelStr = "DEBUG";
        break;
    default:
        levelStr = "UNKNOWN";
        break;
    }
    return levelStr;
}

CMLogger::CMLogger( ICMLogFile& logFile ) :
    logLevel_ ( CM_LOG_LVL_T::CM_LOG_ERROR ),
    logFile_ ( logFile )
{
}

CMLogger::~CMLogger()
{
}

void CMLogger::setLogLevel( CM_LOG_LVL_T logLevel )
{
    if( ( logLevel >= CM_LOG_LVL_T::CM_LOG_ALERT ) && ( logLevel <= CM_LOG_LVL_T::CM_LOG_DEBUG ) ) {
        if( logLevel_ != logLevel ) {
            logLevel_ = logLevel;
            CM_LOG_DEBUG( "Set Debug Level to %d", logLevel_ );
        }
    }
    else {
        CM_LOG_ERROR( "Invalid Debug level %d", logLevel ); 
    }
}

void CMLogger::logMessage( const CM_LOG_LVL_T severity, const bool bIsStrErr, const char* fileName,
    const char* funcName, long lineNumber, const char* message, ... )
{
    if( logLevel_ < severity ) {
        return;
    }

    char logBuf[2048] = { 0 };
    char errStr[2048] = { 0 };
    char logLine[4096] = { 0 };
    va_list  va_args;

    if ( bIsStrErr ) {
        int rc = 0;
        unsigned long lasterr = errno;
#ifdef __USE_GNU
        const char* errorString = strerror_r( errno, errStr, sizeof( errStr ) );
        snprintf( errStr, sizeof( errStr ), "%s" , errorString );        
#else
        rc = strerror_r( errno, errStr, sizeof( errStr ) );
#endif
        if ( 0 != rc ) {
            snprintf( errStr,sizeof( errStr ),"Retrieving error string of %lu failed with error = %d",lasterr,rc );
        }
    }

    snprintf( logBuf, sizeof( logBuf ), "%s:%s:%ld: %s %s", fileName, funcName, lineNumber, message, errStr );
    va_start( va_args, message );
    vsnprintf( logLine, sizeof( logLine ), logBuf, va_args );
    logFile_.writeLogLine( LogLevelStr( severity ), logLine );
    va_end( va_args );
}