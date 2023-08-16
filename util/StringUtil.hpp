/**************************************************************************
 *       Copyright (c) 2023, Cisco Systems, All Rights Reserved
 ***************************************************************************
 *
 *  @file:    StringUtil.hpp
 *
 ***************************************************************************
 * @desc various NSString <-> std::string conversion functions.
 ***************************************************************************/
#pragma once
#include "util.hpp"

#include <string>
#import <Foundation/Foundation.h>

namespace util
{
std::string convertNSStringToStdString(NSString* nsString);
} //namespace util
