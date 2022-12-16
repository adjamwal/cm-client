/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <stdarg.h>
#include <string>
#include <mutex>
#include <filesystem>

enum class CM_LOG_LVL_T
{
    CM_LOG_ALERT = 1,
    CM_LOG_CRITICAL = 2,
    CM_LOG_ERROR = 3,
    CM_LOG_WARNING = 4,
    CM_LOG_NOTICE = 5,
    CM_LOG_INFO = 6,
    CM_LOG_DEBUG = 7
};

class CMLogger
{
public:
	//Exception class to differentiate with standard exceptions
    class logger_exception : public std::runtime_error
    {
    public:
        logger_exception(const std::string& msg) : std::runtime_error(msg) {}
    };

    static CMLogger& getInstance(const std::string& fileName = std::string())
    {
        static CMLogger logger(fileName);
        return logger;
    }

    CMLogger() = delete;
    CMLogger(const CMLogger&) = delete;
    CMLogger& operator=(const CMLogger&) = delete;
    CMLogger(CMLogger &&) = delete;
 
    void logMessage( const CM_LOG_LVL_T severity, const bool bIsStrErr, const char* fileName,
                     const char* funcName, long lineNumber, const char* message, ... );
    void setLogLevel( CM_LOG_LVL_T severity );
    void setLogConfig( uint32_t fileSize, uint32_t logFiles );
private:
    void writeLogLine( const std::string& logLevel, const std::string& logLine );
    bool createLogFile();
    CMLogger(const std::string& fileName);
    ~CMLogger();

    static const uint32_t DEFAULT_MAX_FILE_SIZE = 52428800;
    static const uint32_t DEFAULT_MAX_LOGFILES = 10;  
    CM_LOG_LVL_T logLevel_;
    std::filesystem::path logFileName_;
    std::mutex mutex_;
    uint32_t maxFileSize_;
    uint32_t maxLogFiles_;
    std::string loggerName_; 
};

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define CM_LOG( severity, message, ... ) \
    {CMLogger::getInstance().logMessage( severity, false, __FILENAME__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__ );}

#define CM_LOG_STRERR( severity, message, ... ) \
    {CMLogger::getInstance().logMessage( severity, true, __FILENAME__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__ );}

#define CM_LOG_INIT(FileName)           CMLogger::getInstance(FileName)
#define CM_LOG_ALERT( message, ... )    CM_LOG( CM_LOG_LVL_T::CM_LOG_ALERT, message, ##__VA_ARGS__ )
#define CM_LOG_CRITICAL( message, ... ) CM_LOG( CM_LOG_LVL_T::CM_LOG_CRITICAL, message, ##__VA_ARGS__ )
#define CM_LOG_ERROR( message, ... )    CM_LOG( CM_LOG_LVL_T::CM_LOG_ERROR, message, ##__VA_ARGS__ )
#define CM_LOG_WARNING( message, ... )  CM_LOG( CM_LOG_LVL_T::CM_LOG_WARNING, message, ##__VA_ARGS__ )
#define CM_LOG_NOTICE( message, ... )   CM_LOG( CM_LOG_LVL_T::CM_LOG_NOTICE, message, ##__VA_ARGS__ )
#define CM_LOG_INFO( message, ... )     CM_LOG( CM_LOG_LVL_T::CM_LOG_INFO, message, ##__VA_ARGS__ )
#define CM_LOG_DEBUG( message, ... )    CM_LOG( CM_LOG_LVL_T::CM_LOG_DEBUG, message, ##__VA_ARGS__ )
