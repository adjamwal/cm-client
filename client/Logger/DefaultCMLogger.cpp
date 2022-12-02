/**
 * Default Logger. Only logs to stdout in debug mode
 * Used for unit/component tests
 */

#include "ICMLogger.hpp"
#include <mutex>

class DefaultCMLogger : public ICMLogger
{
public:
    DefaultCMLogger();
    ~DefaultCMLogger();
    virtual void LogMessage( const CM_LOG_LVL_T severity, const bool bIsStrErr, const char* fileName,
    const char* funcName, long lineNumber, const char* message, ... ) override;
    virtual void SetLogLevel( CM_LOG_LVL_T loglevel ) override;
};

static DefaultCMLogger defaultLogger;
static ICMLogger* globalLogger = &defaultLogger;
static std::mutex g_logMutex;

ICMLogger* GetCMLogger()
{
    return globalLogger;
}

void SetCMLogger( ICMLogger* logger )
{
    globalLogger = logger ? logger : &defaultLogger;
}


DefaultCMLogger::DefaultCMLogger()
{
}

DefaultCMLogger::~DefaultCMLogger()
{
}

#ifdef _DEBUG
static void printTime()
{
    char tstr[ 32 ] = {};
    time_t tt;
    struct tm ti;
    time(&tt);
    struct tm ti_obj;
    localtime_r(&tt,&ti_obj);
    ti = ti_obj;

    strftime( tstr, sizeof( tstr ), "%b %d %H:%M:%S", &ti );
    printf( "%s ", tstr );
}
#endif

void DefaultCMLogger::LogMessage( [[maybe_unused]] const CM_LOG_LVL_T severity, 
                                  [[maybe_unused]] const bool bIsStrErr, 
                                  [[maybe_unused]] const char* fileName,
                                  [[maybe_unused]] const char* funcName, 
                                  [[maybe_unused]] long lineNumber, 
                                  [[maybe_unused]] const char* message, ... )
{
#ifdef _DEBUG
    std::lock_guard<std::mutex> lock( g_logMutex );
    printTime();

    va_list args;
    va_start( args, message );
    vprintf( "%s %s %ld %s %s" fileName, funcName, lineNumber, message, args );
    printf( "\n" );
    va_end( args );
    fflush( stdout );
#endif
}

void DefaultCMLogger::SetLogLevel( [[maybe_unused]] CM_LOG_LVL_T severity )
{

}