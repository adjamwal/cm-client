/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <stdarg.h>
#include "CMLogFile.hpp"

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
private:
    CM_LOG_LVL_T logLevel_;
    CMLogFile logFile_;
    const std::string fileName_;
    CMLogger(const std::string& fileName);
    ~CMLogger();
};

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define CM_LOG( severity, message, ... ) \
    {CMLogger::getInstance().logMessage( severity, false, __FILENAME__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__ );}

#define CM_LOG_STRERR( severity, message, ... ) \
    {CMLogger::getInstance().logMessage( severity, true, __FILENAME__, __FUNCTION__, __LINE__, message, ##__VA_ARGS__ );}

#define CM_LOG_INIT(FileName)         CMLogger::getInstance(FileName)
#define CM_LOG_ALERT( message, ... ) CM_LOG( CM_LOG_LVL_T::CM_LOG_ALERT, message, ##__VA_ARGS__ )
#define CM_LOG_CRITICAL( message, ... ) CM_LOG( CM_LOG_LVL_T::CM_LOG_CRITICAL, message, ##__VA_ARGS__ )
#define CM_LOG_ERROR( message, ... ) CM_LOG( CM_LOG_LVL_T::CM_LOG_ERROR, message, ##__VA_ARGS__ )
#define CM_LOG_WARNING( message, ... ) CM_LOG( CM_LOG_LVL_T::CM_LOG_WARNING, message, ##__VA_ARGS__ )
#define CM_LOG_NOTICE( message, ... ) CM_LOG( CM_LOG_LVL_T::CM_LOG_NOTICE, message, ##__VA_ARGS__ )
#define CM_LOG_INFO( message, ... ) CM_LOG( CM_LOG_LVL_T::CM_LOG_INFO, message, ##__VA_ARGS__ )
#define CM_LOG_DEBUG( message, ... ) CM_LOG( CM_LOG_LVL_T::CM_LOG_DEBUG, message, ##__VA_ARGS__ )