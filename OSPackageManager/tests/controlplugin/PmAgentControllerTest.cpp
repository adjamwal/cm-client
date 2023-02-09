#include "gtest/gtest.h"
#include "PmAgentController.hpp"

namespace { // anonymous namespace

#define COMMAND_SHELL_BUFFER_SIZE     1024
const std::string WHITESPACE = " \n\r\t\f\v";

//this function returns the pid if process is running.
//returns -1 otherwise.
int getPIDFromProcessName( const std::string& procName )
{
    int nPID = -1;
    char szCmdBuffer[COMMAND_SHELL_BUFFER_SIZE] = {0};
    std::string command = "pgrep " + procName;
    std::string result;

    FILE *pCmdOutput = popen( command.c_str(), "r" );
    if( NULL == pCmdOutput ) {
        return nPID;
    }
    while( fgets( szCmdBuffer, COMMAND_SHELL_BUFFER_SIZE, pCmdOutput ) ) {
        result += szCmdBuffer;
    }
    pclose( pCmdOutput );

    size_t start = result.find_first_not_of( WHITESPACE );
    result = ( start == std::string::npos ) ? "" : result.substr( start );
    size_t end = result.find_last_not_of( WHITESPACE );
    result = ( end == std::string::npos ) ? "" : result.substr( 0, end + 1 );

    if( !result.empty() ) {
        nPID = atoi( result.c_str() );
    }
    return nPID;
}

bool checkIfProcessIsRunning( const std::string& procName )
{
    return ( -1 != getPIDFromProcessName( procName ) ) ? true : false;
}

}

TEST( PmAgentController, StartStop )
{
    const std::string cmpackagemanagerDir = "../../";

    PmAgentController agentController(cmpackagemanagerDir, "" );
    auto status = agentController.start();
    ASSERT_EQ( PM_STATUS::PM_OK, status );

    //introducing a short sleep of 100ms.
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    ASSERT_EQ( true, checkIfProcessIsRunning( PM_AGENT_BINARY ) );

    status = agentController.stop();
    ASSERT_EQ( PM_STATUS::PM_OK, status );
    ASSERT_EQ( false, checkIfProcessIsRunning( PM_AGENT_BINARY ) );
}

