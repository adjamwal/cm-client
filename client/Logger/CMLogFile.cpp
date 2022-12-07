/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */
#include "CMLogFile.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <chrono>

CMLogFile::CMLogFile(const char* logname) : logFileName_ ( logname ),
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

    std::lock_guard<std::mutex> lock(mutex_);

    createLogFile();
}

CMLogFile::~CMLogFile()
{
    std::lock_guard<std::mutex> lock(mutex_);
    dropLogger();
}

void CMLogFile::writeLogLine( const char* logLevel, const char* logLine )
{
    std::string strLevel(logLevel);
    std::string strLog(logLine);

    spdlog::get(loggerName_)->info(strLevel + ": " + strLog);
    flush();
}

void CMLogFile::setLogConfig( uint32_t fileSize, uint32_t logFiles )
{
    std::lock_guard<std::mutex> lock(mutex_);
    maxFileSize_ = fileSize;
    maxLogFiles_ = logFiles;

    if ( std::filesystem::exists( logFileName_.parent_path() ) ) {
        dropLogger();
        spdlog::rotating_logger_mt( loggerName_, logFileName_.string(), maxFileSize_, maxLogFiles_ - 1 );
    }
}

void CMLogFile::dropLogger()
{
    flush();
    spdlog::drop( loggerName_ );
}

void CMLogFile::flush()
{
    spdlog::get( loggerName_ )->flush();
}

bool CMLogFile::createLogFile()
{
    std::shared_ptr<spdlog::logger> logInstance = NULL;
    if ( !logFileName_.empty() ) {
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


