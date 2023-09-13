/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */
#include "PathUtil.hpp"
#include "StringUtil.hpp"

#import <Foundation/Foundation.h>

namespace util
{

std::string getExecutablePath()
{
    NSString* executablePath = [[NSBundle mainBundle] executablePath];
    return convertNSStringToStdString(executablePath);
}

std::string getApplicationName()
{
    NSString* appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];
    return convertNSStringToStdString(appName);
}

std::string getApplicationVersion()
{
    NSString* appVersion = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    return convertNSStringToStdString(appVersion);
}

} //util

