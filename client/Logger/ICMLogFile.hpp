/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <cstdlib>
#include <cstdint>

class ICMLogFile
{
public:
    ICMLogFile() {};
    virtual ~ICMLogFile() {};
    virtual void Init( const char* logname = NULL ) = 0;
    virtual void WriteLogLine( const char* logLevel, const char* logLine ) = 0;
    virtual void SetLogConfig( uint32_t fileSize, uint32_t logFiles ) = 0;
};


