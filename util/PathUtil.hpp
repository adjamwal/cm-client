/**************************************************************************
 *       Copyright (c) 2023, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 *  @file:    PathUtil.hpp
 *
 ***************************************************************************
 * @desc Fuction that returns path where current executable is located.
 ***************************************************************************/
#pragma once
#include "util.hpp"
#include <string>

namespace util
{
std::string UTIL_MODULE_API getExecutablePath();
std::string UTIL_MODULE_API getApplicationName();
std::string UTIL_MODULE_API getApplicationVersion();
} //namespace util

