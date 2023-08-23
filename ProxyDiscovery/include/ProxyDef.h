#pragma once

#ifdef __APPLE__
    #ifdef PROXY_DISCOVERY_MODULE_API_EXPORTS
        #define PROXY_DISCOVERY_MODULE_API __attribute__((visibility("default")))
    #else
        #define PROXY_DISCOVERY_MODULE_API
    #endif
#else
    #define PROXY_DISCOVERY_MODULE_API
#endif
