/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */
#include "GuidUtil.hpp"
#include "StringUtil.hpp"

#import <Foundation/Foundation.h>

namespace util
{

std::string UTIL_MODULE_API generateGUID()
{
    NSUUID *uuid = [NSUUID UUID];
    NSString *str = [uuid UUIDString];
    return convertNSStringToStdString(str);
}

} //util
