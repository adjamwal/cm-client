#pragma once

#include "ProxyDef.h"
#include <string>

struct PROXY_DISCOVERY_MODULE_API ProxyRecord
{
    std::string url;
    uint32_t port;
    std::string proxyType;

    bool operator ==( const ProxyRecord& rhs ) const
    {
        return ( url == rhs.url ) && ( port == rhs.port ) && ( proxyType != rhs.proxyType );
    }

    bool operator !=( const ProxyRecord& rhs ) const
    {
        return !( *this == rhs );
    }
};
