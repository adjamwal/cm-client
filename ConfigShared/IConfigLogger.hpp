#pragma once

#include "config_shared.hpp"
#include <cstdarg>


#include <filesystem>
#include <string>

namespace ConfigShared
{

class IConfigLogger
{
public:
    virtual ~IConfigLogger() {}
    virtual void Log( int severity, const char* msgFormatter, const char *fileName, const char *funcName, long lineNumber, ... ) = 0;
    virtual void Log( int severity, const char* msgFormatter, const char *fileName, const char *funcName, long lineNumber, va_list args ) = 0;
    virtual void SetLogLevel( int severity ) = 0;
};
} //namespace ConfigShared


#if defined(__FILE_NAME__)
#  define __FILENAME__ __FILE_NAME__
#else
#  define __FILENAME__ std::filesystem::path(__FILE__).filename().string().c_str()
#endif

#ifndef __FUNCTION__
#   define __FUNCTION__ __func__
#endif

#define CONFIG_LOG_ALERT( fmt, ... ) getConfigLogger()->Log( 1, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define CONFIG_LOG_CRITICAL( fmt, ... ) getConfigLogger()->Log( 2, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define CONFIG_LOG_ERROR( fmt, ... ) getConfigLogger()->Log( 3, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define CONFIG_LOG_WARNING( fmt, ... ) getConfigLogger()->Log( 4, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define CONFIG_LOG_NOTICE( fmt, ... ) getConfigLogger()->Log( 5, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define CONFIG_LOG_INFO( fmt, ... ) getConfigLogger()->Log( 6, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define CONFIG_LOG_DEBUG( fmt, ... ) getConfigLogger()->Log( 7, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
