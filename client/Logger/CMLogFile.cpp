/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */
#include "CMLogFile.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <chrono>

CMLogFile::CMLogFile() : logFileName_ ( "" ),
                         maxFileSize_ ( DEFAULT_MAX_FILE_SIZE ),
                         maxLogFiles_ ( DEFAULT_MAX_LOGFILES ),
                         loggerName_ ( "rot_log" )
{
    class trace_formatter : public spdlog::custom_flag_formatter
    {
    public:
        void format( const spdlog::details::log_msg&, const std::tm&, spdlog::memory_buf_t& dest ) override
        {
            std::chrono::milliseconds millisec = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() );
            unsigned long ms = static_cast<unsigned int>( millisec.count() );
            std::string tickStr = std::to_string( ms );
            dest.append(tickStr.data(), tickStr.data() + tickStr.size());
        }

        std::unique_ptr< custom_flag_formatter > clone() const override
        {
            return spdlog::details::make_unique< trace_formatter >();
        }
    };

    auto formatter = std::make_unique< spdlog::pattern_formatter >();
    formatter->add_flag< trace_formatter >( '*' ).set_pattern( "(%*, +%o ms) %b %d %T [%t] %v" );
    spdlog::set_formatter( std::move(formatter) );
}

CMLogFile::~CMLogFile()
{
    deinit();
}

void CMLogFile::writeLogLine( const char* logLevel, const char* logLine )
{
    init(); // Takes mutex

    std::string strLevel(logLevel);
    std::string strLog(logLine);

    if ( isLoggerInitialized() ) {
        spdlog::get(loggerName_)->info(strLevel + ": " + strLog);
        flush();
    }
    else {
        spdlog::error("failed to log ({})", strLog);
    }
}

void CMLogFile::init( const char* logname )
{
    std::lock_guard<std::mutex> lock(mutex_);

    if ( !isLoggerInitialized() ) {
        if ( logname ) {
        logFileName_ = logname;
        }
        createLogFile();
    }
}

void CMLogFile::setLogConfig( uint32_t fileSize, uint32_t logFiles )
{
    std::lock_guard<std::mutex> lock(mutex_);
    maxFileSize_ = fileSize;
    maxLogFiles_ = logFiles;

    if ( isLoggerInitialized() && std::filesystem::exists( logFileName_.parent_path() ) ) {
        dropLogger();
        spdlog::rotating_logger_mt( loggerName_, logFileName_.string(), maxFileSize_, maxLogFiles_ - 1 );
    }
}

void CMLogFile::deinit()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ( isLoggerInitialized() ) {
        dropLogger();
    }
}

bool CMLogFile::isLoggerInitialized()
{
    return !( spdlog::get( loggerName_ ) == NULL );
}

void CMLogFile::dropLogger()
{
    flush();
    spdlog::drop( loggerName_ );
}

void CMLogFile::flush()
{
    if (isLoggerInitialized()) {
        spdlog::get( loggerName_ )->flush();
    }
}

bool CMLogFile::createLogFile()
{
    std::shared_ptr<spdlog::logger> logInstance = NULL;
    if ( !isLoggerInitialized() && !logFileName_.empty() ) {
        try
        {
            if ( !std::filesystem::exists( logFileName_.parent_path() ) ) {
                std::filesystem::create_directories( logFileName_.parent_path() );
            }
            logInstance = spdlog::rotating_logger_mt( loggerName_, logFileName_.string(), maxFileSize_, maxLogFiles_ - 1 );
        }
        catch( ... )
        {
            return false;
        }
    }
    return logInstance ? true : false;
}


