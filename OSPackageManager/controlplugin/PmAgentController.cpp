#include "PmAgentController.hpp"
#include <iostream>
#include <vector>
#include <libproc.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#define RESTART_DELAY_CHRONO 30s

using namespace std::chrono_literals;

PmAgentController::PmAgentController( const std::string& path, const std::string& configPath ) :
      processPath_( path + "/" + PM_AGENT_BINARY ) 
    , bsConfigPath_( configPath + "/" + BS_CONFIG_FILE )
    , pmConfigPath_( configPath + "/" + PM_CONFIG_FILE )
    , bIsProcessStartedByPlugin_( false )
{
    if( path.empty() ) {
        throw std::invalid_argument( "AgentController basepath has not been set" );
    }
}

PmAgentController::~PmAgentController()
{
    Stop();
    cleanup();
}

PM_STATUS PmAgentController::Start()
{
    //check if previous process is running
    // and kill it.
    if( PM_STATUS::PM_OK != killIfRunning() ) {
        return PM_STATUS::PM_FAIL;
    }

    //start process.
    if( PM_STATUS::PM_OK != startProcess() ) {
        return PM_STATUS::PM_FAIL;
    }

    bIsProcessStartedByPlugin_ = true;

    //launch thread to monitor process.
    threadMonitor_ = std::thread( &PmAgentController::monitorProcess, this );
    return PM_STATUS::PM_OK;
}

PM_STATUS PmAgentController::Stop()
{
    auto status = PM_STATUS::PM_ERROR;
    {
        std::lock_guard<std::mutex> lock( mutex_ );
        if( bIsProcessStartedByPlugin_ )
        {
            bIsProcessStartedByPlugin_ = false;
            if( PM_STATUS::PM_OK != stopProcess() )
            {
                return PM_STATUS::PM_FAIL;
            }
        }
    }
    //Wait for monitor thread to exit
    if( threadMonitor_.joinable() )
    {
        threadMonitor_.join();
    }
    return PM_STATUS::PM_OK;
}

void PmAgentController::monitorProcess()
{
    while( eProcess_Terminated == waitForProcess() ) {
        std::lock_guard<std::mutex> lock( mutex_ );
        //check if the process was terminated by us
        //and if yes, stop monitoring.
        if( !bIsProcessStartedByPlugin_ ) {
            break;
        }

        std::this_thread::sleep_for( RESTART_DELAY_CHRONO );
        startProcess();
    }
}

void PmAgentController::cleanup()
{
    pid_ = -1;
}

PmAgentController::eProcStatus PmAgentController::waitForProcess()
{
    int iChildStatus = 0;
    if ( 0 != waitpid( pid_, &iChildStatus, 0 ) ) {
        pid_ = INVALID_PID;
        return eProcess_Terminated;
    }
    return eProcess_Active;
}

PM_STATUS PmAgentController::killIfRunning()
{
    int iProcessCount = proc_listpids( PROC_ALL_PIDS, 0, NULL, 0 );
    pid_t processIDs[iProcessCount];
    proc_listpids( PROC_ALL_PIDS, 0, processIDs, sizeof( processIDs ) );
    for ( int iProc = 0; iProc < iProcessCount; iProc++ )
    {
        struct proc_bsdinfo stProcInfo;
        if ( PROC_PIDTBSDINFO_SIZE == proc_pidinfo( processIDs[iProc], PROC_PIDTBSDINFO, 0, &stProcInfo, PROC_PIDTBSDINFO_SIZE ) ) {
            if( PM_AGENT_BINARY == std::string( stProcInfo.pbi_name ) ) {
                if ( 0 == kill( processIDs[iProc], SIGTERM ) ) {
                    break;
                }
                return PM_STATUS::PM_ERROR;
            }
        }
    }
    return PM_STATUS::PM_OK;
}

PM_STATUS PmAgentController::startProcess()
{
    PM_STATUS status = PM_STATUS::PM_ERROR;
    /* TODO:
    ** Verify CodeSign
    */

    if ( INVALID_PID != pid_ ) {
        return PM_STATUS::PM_ERROR;
    }

    std::vector<char *> vszProcessArgs = {
        strdup( processPath_.c_str() ),
        strdup( "--bootstrap" ),
        strdup( bsConfigPath_.c_str() ),
        strdup( "--config-file" ),
        strdup( pmConfigPath_.c_str() ),
        NULL
    };
    pid_ = fork();
    switch ( pid_ ) {
        case -1:
            // Error
            status = PM_STATUS::PM_ERROR;
            goto safe_exit;
        case 0:
            // Child
            if(0 != execv( vszProcessArgs[0], vszProcessArgs.data()) )
            {
                exit( errno );
            }
        default:
            status = PM_STATUS::PM_ERROR;
            // Parent
    }
safe_exit:
    return status;
}

PM_STATUS PmAgentController::stopProcess()
{
    if( INVALID_PID == pid_ ) {
        return PM_STATUS::PM_OK;
    }

    if( 0 == kill( pid_, SIGTERM ) ) {
        pid_ = INVALID_PID;
        return PM_STATUS::PM_OK;
    }
    return PM_STATUS::PM_ERROR;
}
