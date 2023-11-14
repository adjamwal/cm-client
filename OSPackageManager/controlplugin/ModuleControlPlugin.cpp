
#include "ModuleControlPlugin.hpp"
#include "PmAgentController.hpp"
#include "ProcessWrapper.hpp"
#include "CMLogger.hpp"
#include <iostream>
#include <stddef.h> 

namespace { // anonymous namespace

class PmControlPlugin
{
public:
    // static member functions

    static PmControlPlugin&
    getInstance( const std::string& basePath = std::string(""), const std::string& configPath = std::string("") );

    static PM_MODULE_RESULT_T
    startPmAgent( const TCHAR* basePath, const TCHAR* dataPath, const TCHAR* configPath );

    static PM_MODULE_RESULT_T
    stopPmAgent();

    static PM_MODULE_RESULT_T
    setPmOption( PM_MODULE_OPTION_ID_T optionID, void* option, size_t opSize );
private:
    // private member functions

    PmControlPlugin( const std::string& basePath, const std::string& configPath ) : agentCtrlInst_( basePath, configPath, std::make_shared<ProcessWrapper>() ),
        bIsProcessStarted_( false )
    {
    }

    // prevent all copy and move construction and assignments
    PmControlPlugin( PmControlPlugin&& ) = delete;

    // private data members
    PmAgentController agentCtrlInst_;
    std::mutex mtxAgentCtrl_; // protect instance returned by getInstance()
    bool bIsProcessStarted_;  // tracking bool for module Start/Stop
};

PmControlPlugin&
PmControlPlugin::getInstance( const std::string& basepath, const std::string& configpath )
{
    static PmControlPlugin s_ctrlPluginInstance( basepath, configpath );
    return s_ctrlPluginInstance;
}

PM_MODULE_RESULT_T
to_pm_result( const PM_STATUS eStatus )
{
    switch ( eStatus ) {
    case PM_STATUS::PM_OK: return PM_MODULE_SUCCESS;
    // TODO: define more mappings
    default:               return PM_MODULE_GENERAL_ERROR;
    }
}

PM_MODULE_RESULT_T
PmControlPlugin::startPmAgent( const TCHAR* basePath,
                               const TCHAR* dataPath,
                               const TCHAR* configPath )
{
    CM_LOG_DEBUG( "Starting PM agent" );
    try {
        PmControlPlugin& ctrlPlugin = getInstance( basePath, configPath );
        std::lock_guard<std::mutex> lck( ctrlPlugin.mtxAgentCtrl_ );
        if ( ctrlPlugin.bIsProcessStarted_ ) {
            return PM_MODULE_ALREADY_STARTED;
        }
        
        PM_STATUS retStatus = ctrlPlugin.agentCtrlInst_.start();
        if ( PM_STATUS::PM_OK == retStatus ) {
            ctrlPlugin.bIsProcessStarted_ = true;
            return PM_MODULE_SUCCESS;
        }
        else {
            CM_LOG_ERROR( "Failed to start agent with return code [%d]", retStatus );
            return to_pm_result( retStatus );
        }
    } catch ( const std::exception &rLogEx ) {
        std::cerr << "Fatal error: " << rLogEx.what() << std::endl;
    }
    return PM_MODULE_GENERAL_ERROR;
}

PM_MODULE_RESULT_T
PmControlPlugin::stopPmAgent()
{
    try
    {
        PmControlPlugin& ctrlPlugin = getInstance();
        std::lock_guard<std::mutex> lck( ctrlPlugin.mtxAgentCtrl_ );
        if ( !ctrlPlugin.bIsProcessStarted_ )
        {
            return PM_MODULE_NOT_STARTED;
        }

        PM_STATUS retStatus = ctrlPlugin.agentCtrlInst_.stop();
        if ( PM_STATUS::PM_OK == retStatus )
        {
            ctrlPlugin.bIsProcessStarted_ = false;
            return PM_MODULE_SUCCESS;
        }

        CM_LOG_ERROR( "Failed to stop agent with return code [%d]", retStatus );

        return to_pm_result( retStatus );
    }
    catch ( const std::exception& e )
    {
        CM_LOG_WARNING( "package manager agent not started: %s", e.what() );
        return PM_MODULE_NOT_STARTED;
    }
}

PM_MODULE_RESULT_T 
PmControlPlugin::setPmOption( PM_MODULE_OPTION_ID_T optionID, void* option, size_t opSize )
{
    try
    {
        if( optionID == PM_MODULE_OPTION_LOG_LEVEL ) {
           // TODO set log level
        }
        else {
            throw( "Invalid PM option" );
            return PM_MODULE_SUCCESS;
        }
    }
    catch( const std::exception& e ) {
        CM_LOG_WARNING( "Invalid option parameter: %s", e.what() );
        return PM_MODULE_INVALID_PARAM;
    }

    return PM_MODULE_SUCCESS;
}

} // end anonymous namespace

PM_MODULE_API
PM_MODULE_RESULT_T
CreatePMModuleInstance( IN OUT PM_MODULE_CTX_T* pPM_MODULE_CTX )
{
    if ( nullptr == pPM_MODULE_CTX ) {
        return PM_MODULE_INVALID_PARAM;
    }

    // NOTE: not doing anything with PmControlPlugin singleton here
    // NOTE: not complaining here about any paths; will only know about paths in Start

    pPM_MODULE_CTX->fpInit   = nullptr;
    pPM_MODULE_CTX->fpDeinit = nullptr;
    pPM_MODULE_CTX->fpStart  = PmControlPlugin::startPmAgent;
    pPM_MODULE_CTX->fpStop   = PmControlPlugin::stopPmAgent;
    pPM_MODULE_CTX->fpSetOption = PmControlPlugin::setPmOption;
    pPM_MODULE_CTX->fpConfigUpdated = nullptr;
    return PM_MODULE_SUCCESS;
}

PM_MODULE_API
PM_MODULE_RESULT_T
ReleasePMModuleInstance( IN OUT PM_MODULE_CTX_T* pPM_MODULE_CTX )
{
    if ( nullptr == pPM_MODULE_CTX ) {
        return PM_MODULE_INVALID_PARAM;
    }

    // NOTE: not doing anything with PmControlPlugin singleton here

    pPM_MODULE_CTX->fpStart = nullptr;
    pPM_MODULE_CTX->fpStop  = nullptr;
    pPM_MODULE_CTX->fpSetOption = nullptr;
    return PM_MODULE_SUCCESS;
}
