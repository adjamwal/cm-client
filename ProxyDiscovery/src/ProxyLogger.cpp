/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "IProxyLogger.h"

namespace {
    class DefaultProxyLogger: public proxy::IProxyLogger
    {
    public:
        void Log( int severity, const char* msgFormatter, ... ) override {}
        void Log( int severity, const char* msgFormatter, va_list args ) override {}
	};

    DefaultProxyLogger g_defaultLogger;
    proxy::IProxyLogger* g_proxyLogger;
}

namespace proxy {

IProxyLogger& GetProxyLogger()
{
    return g_proxyLogger ? *g_proxyLogger : g_defaultLogger;
}

void PROXY_DISCOVERY_MODULE_API SetProxyLogger(IProxyLogger* logger)
{
    g_proxyLogger = logger;
}

} //namespace proxy
