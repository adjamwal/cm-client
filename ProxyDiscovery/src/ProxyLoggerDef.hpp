#pragma once

#include "IProxyLogger.h"

#include <filesystem>
#include <string>

#if defined(__FILE_NAME__)
#  define __FILENAME__ __FILE_NAME__
#else
#  define __FILENAME__ std::filesystem::path(__FILE__).filename().string().c_str()
#endif

#ifndef __FUNCTION__
#   define __FUNCTION__ __func__
#endif

#define PROXY_LOG_ALERT( fmt, ... ) proxy::GetProxyLogger().Log( 1, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define PROXY_LOG_CRITICAL( fmt, ... ) proxy::GetProxyLogger().Log( 2, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define PROXY_LOG_ERROR( fmt, ... ) proxy::GetProxyLogger().Log( 3, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define PROXY_LOG_WARNING( fmt, ... ) proxy::GetProxyLogger().Log( 4, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define PROXY_LOG_NOTICE( fmt, ... ) proxy::GetProxyLogger().Log( 5, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define PROXY_LOG_INFO( fmt, ... ) proxy::GetProxyLogger().Log( 6, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )

#define PROXY_LOG_DEBUG( fmt, ... ) proxy::GetProxyLogger().Log( 7, "%s:%s:%d: " fmt, __FILENAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
