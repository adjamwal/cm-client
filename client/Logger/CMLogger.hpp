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
#include "IConfigLogger.hpp"

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
    
    void Log(CM_LOG_LVL_T severity, bool bIsStrErr, const char *message, ... );
    void Log(CM_LOG_LVL_T severity, bool bIsStrErr, const char *message, va_list args);
    
    void SetLogLevel( CM_LOG_LVL_T severity );
    bool setLogConfig( uint32_t fileSize, uint32_t logFiles );

    ConfigShared::IConfigLogger& getConfigLogger();

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
    
    class ConfigLogger: public ConfigShared::IConfigLogger
    {
    public:
        explicit ConfigLogger(CMLogger* cLogger);
        ConfigLogger(const ConfigLogger&) = delete;
        ConfigLogger(ConfigLogger&&) = delete;
        ConfigLogger& operator =(const ConfigLogger&) = delete;
        ConfigLogger& operator =(ConfigLogger&&) = delete;
        void Log( int severity, const char* msgFormatter, const char *fileName, const char *funcName, long lineNumber,  ... ) override;
        void Log( int severity, const char* msgFormatter, const char *fileName, const char *funcName, long lineNumber, va_list args ) override;
        void SetLogLevel( int severity ) override;
    private:
        CMLogger* cOrigLogger_;
    };

    ConfigLogger configLogger_;
};

#define CM_LOG( severity, fmt, ... ) \
    {CMLogger::getInstance().Log( severity, false, fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ );}

#define CM_LOG_STRERR( severity, fmt, ... ) \
    {CMLogger::getInstance().Log( severity, true, fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ );}

#define CM_LOG_INIT(FileName)           CMLogger::getInstance(FileName)
#define CM_LOG_ALERT( fmt, ... )    CM_LOG( CM_LOG_LVL_T::CM_LOG_ALERT, "%s:%s:%d: " fmt, ##__VA_ARGS__ )
#define CM_LOG_CRITICAL( fmt, ... ) CM_LOG( CM_LOG_LVL_T::CM_LOG_CRITICAL, "%s:%s:%d: " fmt, ##__VA_ARGS__ )
#define CM_LOG_ERROR( fmt, ... )    CM_LOG( CM_LOG_LVL_T::CM_LOG_ERROR, "%s:%s:%d: " fmt, ##__VA_ARGS__ )
#define CM_LOG_WARNING( fmt, ... )  CM_LOG( CM_LOG_LVL_T::CM_LOG_WARNING, "%s:%s:%d: " fmt, ##__VA_ARGS__ )
#define CM_LOG_NOTICE( fmt, ... )   CM_LOG( CM_LOG_LVL_T::CM_LOG_NOTICE, "%s:%s:%d: " fmt, ##__VA_ARGS__ )
#define CM_LOG_INFO( fmt, ... )     CM_LOG( CM_LOG_LVL_T::CM_LOG_INFO, "%s:%s:%d: " fmt, ##__VA_ARGS__ )
#define CM_LOG_DEBUG( fmt, ... )    CM_LOG( CM_LOG_LVL_T::CM_LOG_DEBUG, "%s:%s:%d: " fmt, ##__VA_ARGS__ )
