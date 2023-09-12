/**************************************************************************
 *       Copyright (c) 2023, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 *  @file:    config_shared.hpp
 *
 ***************************************************************************
 * @desc Contains definition of the CONFIGSHARED_MODULE_API macro.
 	The purpose of this macro is to export symbols from the library.
 ***************************************************************************/
#pragma once

#ifdef __APPLE__
    #ifdef CONFIGSHARED_MODULE_API_EXPORTS
        #define CONFIGSHARED_MODULE_API __attribute__((visibility("default")))
    #else
        #define CONFIGSHARED_MODULE_API
    #endif
#else
    #define CONFIGSHARED_MODULE_API
#endif
