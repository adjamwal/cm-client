/**************************************************************************
 *       Copyright (c) 2023, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 *  @file:    util.hpp
 *
 ***************************************************************************
 * @desc Contains definition of the UTIL_MODULE_API macro.
 	The purpose of this macro is to export symbols from the library.
 ***************************************************************************/
#pragma once

#ifdef __APPLE__
    #ifdef UTIL_MODULE_API_EXPORTS
        #define UTIL_MODULE_API __attribute__((visibility("default")))
    #else
        #define UTIL_MODULE_API
    #endif
#else
    #define UTIL_MODULE_API
#endif