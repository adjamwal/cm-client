/**
 * @file
 *
 * @copyright (c) 2022 Cisco Systems, Inc. All rights reserved
 */
#include "CMLogFile.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <chrono>

CMLogFile::CMLogFile() :
    m_logFileName( "" )
    , m_maxFileSize( DEFAULT_MAX_FILE_SIZE )
    , m_maxLogFiles( DEFAULT_MAX_LOGFILES )
    , m_loggerName ( "rot_log" )
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
    Deinit();
}

void CMLogFile::WriteLogLine( const char* logLevel, const char* logLine )
{
    Init(); // Takes mutex

    std::string strLevel(logLevel);
    std::string strLog(logLine);

    if ( IsLoggerInitialized() ) {
        spdlog::get(m_loggerName)->info(strLevel + ": " + strLog);
        Flush();
    }
    else {
        spdlog::error("failed to log ({})", strLog);
    }
}

void CMLogFile::Init( const char* logname )
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if ( !IsLoggerInitialized() ) {
        if ( logname ) {
        m_logFileName = logname;
        }
        CreateLogFile();
    }
}

void CMLogFile::SetLogConfig( uint32_t fileSize, uint32_t logFiles )
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_maxFileSize = fileSize;
    m_maxLogFiles = logFiles;

    if ( IsLoggerInitialized() && std::filesystem::exists( m_logFileName.parent_path() ) ) {
        DropLogger();
        spdlog::rotating_logger_mt( m_loggerName, m_logFileName.string(), m_maxFileSize, m_maxLogFiles - 1 );
    }
}

void CMLogFile::Deinit()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if ( IsLoggerInitialized() ) {
        DropLogger();
    }
}

bool CMLogFile::IsLoggerInitialized()
{
    return !( spdlog::get( m_loggerName ) == NULL );
}

void CMLogFile::DropLogger()
{
    Flush();
    spdlog::drop( m_loggerName );
}

void CMLogFile::Flush()
{
    if (IsLoggerInitialized()) {
        spdlog::get( m_loggerName )->flush();
    }
}

bool CMLogFile::CreateLogFile()
{
    std::shared_ptr<spdlog::logger> logInstance = NULL;
    if ( !IsLoggerInitialized() && !m_logFileName.empty() ) {
        try
        {
            if ( !std::filesystem::exists( m_logFileName.parent_path() ) ) {
                std::filesystem::create_directories( m_logFileName.parent_path() );
            }
            logInstance = spdlog::rotating_logger_mt( m_loggerName, m_logFileName.string(), m_maxFileSize, m_maxLogFiles - 1 );
        }
        catch( ... )
        {
            return false;
        }
    }
    return logInstance ? true : false;
}


