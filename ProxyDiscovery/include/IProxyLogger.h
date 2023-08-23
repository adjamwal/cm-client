#pragma once

#include "ProxyDef.h"
#include <cstdarg>


namespace proxy
{
class IProxyLogger
{
public:
    virtual ~IProxyLogger() {}
    virtual void Log( int severity, const char* msgFormatter, ... ) = 0;
    virtual void Log( int severity, const char* msgFormatter, va_list args ) = 0;
};

IProxyLogger& GetProxyLogger();
void PROXY_DISCOVERY_MODULE_API SetProxyLogger(IProxyLogger* logger);

} //namespace proxy
