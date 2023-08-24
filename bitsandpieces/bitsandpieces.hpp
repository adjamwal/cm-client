/**************************************************************************
 *       Copyright (c) 2023, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 *  @file:    bitsandpieces.hpp
 *
 ***************************************************************************
 * @desc Contains definition of the BITSANDPIECES_MODULE_API macro.
 	The purpose of this macro is to export symbols from the library.
 ***************************************************************************/
#pragma once

#ifdef __APPLE__
    #ifdef BITSANDPIECES_MODULE_API_EXPORTS
        #define BITSANDPIECES_MODULE_API __attribute__((visibility("default")))
    #else
        #define BITSANDPIECES_MODULE_API
    #endif
#else
    #define BITSANDPIECES_MODULE_API
#endif
