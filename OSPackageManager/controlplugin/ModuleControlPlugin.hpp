#pragma once

#include "cmid/PackageManagerInternalModuleAPI.h"
/*
* This header is an extension of PackageManagerInternalModuleAPI.h and is being created to ensure both cmid and pm components are loaded
without any ambiguity using the same API interface when both their respective control plugins are built statically.
*/

/**
* @brief Creates a PM module instance - REQUIRED
*
* @param[in,out] pPM_MODULE_CTX  pointer to module context object. Caller should set version field of the context
*                            to an expected interface version. If that version is supported by the module then the module will populate
*                            the remaining fields of the context based on that expected version.
*
* @return PM_MODULE_SUCCESS in case of success, <br>
*         PM_MODULE_UNSUPPORTED_VERSION if requested interface version is not supported by the module, <br>
*         PM_MODULE_UNSUPPORTED_PLATFORM_VERSION if current platform is not supported by the module, <br>
*         other PM_MODULE_RESULT_T in case of errors. <br>
*/
extern "C" PM_MODULE_RESULT_T PM_MODULE_API CreatePMModuleInstance(IN OUT PM_MODULE_CTX_T* pPM_MODULE_CTX);

/**
* @brief Releases a PM module instance - REQUIRED
*
* This API should be called for each call of CreatePMModuleInstance
*
* @param[in,out] pPM_MODULE_CTX  pointer to module context object
*
* @return PM_MODULE_SUCCESS in case of success, <br>
*         other PM_MODULE_RESULT_T in case of errors. <br>
*/
extern "C" PM_MODULE_RESULT_T PM_MODULE_API ReleasePMModuleInstance(IN OUT PM_MODULE_CTX_T* pPM_MODULE_CTX);