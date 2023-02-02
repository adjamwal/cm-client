/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */
#include "CMLogger.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <chrono>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

std::string LogLevelStr( const CM_LOG_LVL_T level )
{
    std::string levelStr;
    switch ( level ) {
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

CMLogger::CMLogger( const std::string& fileName ) :
    logLevel_ ( CM_LOG_LVL_T::CM_LOG_DEBUG ),
    logFileName_ ( fileName ),
    maxFileSize_ ( DEFAULT_MAX_FILE_SIZE ),
    maxLogFiles_ ( DEFAULT_MAX_LOGFILES ),
    loggerName_ ( "rot_log" )  
{
    if ( logFileName_.empty() ) {
        throw logger_exception( "Empty log filename" );
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if ( false == createLogFile() ) {
        throw logger_exception( "Logger creation failed" );
    }
}

CMLogger::~CMLogger()
{
    std::lock_guard<std::mutex> lock(mutex_);
    spdlog::get( loggerName_ )->flush();
    spdlog::drop( loggerName_ );
}

void CMLogger::setLogLevel( CM_LOG_LVL_T logLevel )
{
    if ( ( logLevel >= CM_LOG_LVL_T::CM_LOG_ALERT ) && ( logLevel <= CM_LOG_LVL_T::CM_LOG_DEBUG ) ) {
        if ( logLevel_ != logLevel ) {
            logLevel_ = logLevel;
            CM_LOG_DEBUG( "Set Debug Level to %d", logLevel_ );
        }
    } else {
        CM_LOG_ERROR( "Invalid Debug level %d", logLevel ); 
    }
}

void CMLogger::logMessage( const CM_LOG_LVL_T severity, const bool bIsStrErr, const char *fileName,
    const char *funcName, long lineNumber, const char *message, ... )
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
    writeLogLine( LogLevelStr( severity ), logLine );
    va_end( va_args );
}

void CMLogger::writeLogLine( const std::string& logLevel, const std::string& logLine )
{
    spdlog::get(loggerName_)->info(logLevel + ": " + logLine);
    spdlog::get( loggerName_ )->flush();
}

bool CMLogger::setLogConfig( uint32_t fileSize, uint32_t logFiles )
{
    bool status = false;
    std::lock_guard<std::mutex> lock(mutex_);
    maxFileSize_ = fileSize;
    maxLogFiles_ = logFiles;

    try {
        if ( std::filesystem::exists( logFileName_.parent_path() ) ) {
            spdlog::get( loggerName_ )->flush();
            spdlog::drop( loggerName_ );
        
            if( NULL != spdlog::rotating_logger_mt( loggerName_, logFileName_.string(), maxFileSize_, maxLogFiles_ - 1 ) ) {
                status = true;
            }
        }
    }
    catch( ... ) {
        status = false;
    }
    
    return status;
}

bool CMLogger::createLogFile()
{
    class trace_formatter : public spdlog::custom_flag_formatter
    {
    public:
        void format( const spdlog::details::log_msg&, const std::tm&, spdlog::memory_buf_t& dest ) override
        {
            std::chrono::milliseconds millisec = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() );
            unsigned long ms = static_cast<unsigned int>( millisec.count() );
            std::string tickStr = std::to_string( ms );
            dest.append(tickStr.data(), tickStr.data() + tickStr.size());
        }

        std::unique_ptr< custom_flag_formatter > clone() const override
        {
            return spdlog::details::make_unique< trace_formatter >();
        }
    };

    auto formatter = std::make_unique< spdlog::pattern_formatter >();
    formatter->add_flag< trace_formatter >( '*' ).set_pattern( "(%*, +%o ms) %b %d %T [%t] %v" );
    spdlog::set_formatter( std::move(formatter) );

    std::shared_ptr<spdlog::logger> logInstance = NULL;

    try {
        if ( !std::filesystem::exists( logFileName_.parent_path() ) ) {
            std::filesystem::create_directories( logFileName_.parent_path() );
        }
        logInstance = spdlog::rotating_logger_mt( loggerName_, logFileName_.string(), maxFileSize_, maxLogFiles_ - 1 );
    }
    catch( ... ) {
        throw logger_exception( "Failed to create/open logger : " + logFileName_.string() );
    }

    return ( NULL != logInstance ) ? true : false;
}