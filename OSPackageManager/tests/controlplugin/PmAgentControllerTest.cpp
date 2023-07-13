/**
 * @file
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "PmAgentController.hpp"
#include "ProcessWrapper.hpp"
#include "FakeProcessWrapper.hpp"
#include "ExecutionError.hpp"

#include <memory>
#include <system_error>

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

//TODO: Extract to some common space
std::error_code errno_to_error_code(int errnoValue) {
    static const std::error_category& errnoCategory = std::generic_category();
    return std::error_code(errnoValue, errnoCategory);
}

} //unnamed namespace

using ::testing::ElementsAre;
using namespace std::chrono_literals;

TEST( PmAgentController, StartStop )
{
    PmAgentController agentController(CMID_DAEMON_PATH, "", std::make_shared<ProcessWrapper>() );
    auto status = agentController.start();
    ASSERT_EQ( PM_STATUS::PM_OK, status );

    //introducing a short sleep of 100ms.
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    ASSERT_EQ( true, checkIfProcessIsRunning( PM_AGENT_BINARY ) );

    status = agentController.stop();
    ASSERT_EQ( PM_STATUS::PM_OK, status );
    ASSERT_EQ( false, checkIfProcessIsRunning( PM_AGENT_BINARY ) );
}

class TestPmAgentController: public ::testing::Test
{
public:
    TestPmAgentController()
    {
        pProcessWrapper_ = std::make_shared<FakeProcessWrapper>(PM_AGENT_BINARY);
        pController_ = std::make_unique<PmAgentController>(CMID_DAEMON_PATH, "", pProcessWrapper_, 0ms);
    }
    
protected:
    std::unique_ptr<PmAgentController> pController_;
    std::shared_ptr<FakeProcessWrapper> pProcessWrapper_;
};

TEST_F(TestPmAgentController, KillIsCalled)
{
    pProcessWrapper_->createProcess("SomeProcess1");
    pProcessWrapper_->createProcess(PM_AGENT_BINARY);
    pProcessWrapper_->createProcess("SomeProcess2");
    
    ASSERT_EQ(pController_->start(), PM_STATUS::PM_OK);
    ASSERT_TRUE(pController_->waitMonitorThreadInitialized());
    pController_->stop();
    const std::deque<pid_t>& killedProceses = pProcessWrapper_->getProcessedKillCalls();
    ASSERT_THAT(killedProceses, ElementsAre(2, 4));
}

TEST_F(TestPmAgentController, ForkGeneratesExceptionOneTime)
{
    //Add error generation for the fork call
    pProcessWrapper_->createProcess("SomeProcess1");
    ASSERT_EQ(pController_->start(), PM_STATUS::PM_OK);
    ASSERT_TRUE(pController_->waitMonitorThreadInitialized());
    pProcessWrapper_->addForkCall([]() -> pid_t {
        std::error_code ec = errno_to_error_code(ENOMEM);
        throw std::system_error(ec);
        return 1;
    });
    std::vector<pid_t> processes = pProcessWrapper_->getRunningProcesses();
    ASSERT_EQ(2, processes.size());
    ASSERT_EQ(2, processes[1]);
    pProcessWrapper_->kill(2, IProcessWrapper::EWaitForProcStatus::ProcessTerminated);
    
    ASSERT_TRUE(pController_->waitForMonitorIteration(2));
    
    //First fork in pController_->start(), second and third in
    //PmAgentController::monitorProcess() thread call.
    ASSERT_EQ(3, pProcessWrapper_->getNumberOfForkCalls());
    processes = pProcessWrapper_->getRunningProcesses();
    ASSERT_EQ(2, processes.size());
    //2 pid is killed in pProcessWrapper_->kill, 3 pid must be created then.
    ASSERT_EQ(3, processes[1]);
    pController_->stop();
}

TEST_F(TestPmAgentController, StartFailedIfUnableToStartProcess)
{
    pProcessWrapper_->addForkCall([]() -> pid_t {
        std::error_code ec = errno_to_error_code(ENOMEM);
        throw std::system_error(ec);
        return 1;
    });
    ASSERT_EQ(pController_->start(), PM_STATUS::PM_FAIL);
}

TEST_F(TestPmAgentController, TestImpossibleWaitStatusProduced)
{
    ASSERT_EQ(pController_->start(), PM_STATUS::PM_OK);
    ASSERT_TRUE(pController_->waitMonitorThreadInitialized());
    std::vector<pid_t> processes = pProcessWrapper_->getRunningProcesses();
    ASSERT_EQ(1, processes.size());
    ASSERT_EQ(1, processes[0]);
    pProcessWrapper_->kill(1, IProcessWrapper::EWaitForProcStatus::ImpossibleError);
    //monitor thread should stop its execution
    //test will hung otherwise
    pController_->waitMonitorThreadStopped();
}

TEST_F(TestPmAgentController, TestProcessNotAChildWaitStatusProduced)
{
    ASSERT_EQ(pController_->start(), PM_STATUS::PM_OK);
    ASSERT_TRUE(pController_->waitMonitorThreadInitialized());
    std::vector<pid_t> processes = pProcessWrapper_->getRunningProcesses();
    ASSERT_EQ(1, processes.size());
    ASSERT_EQ(1, processes[0]);
    pProcessWrapper_->kill(1, IProcessWrapper::EWaitForProcStatus::ProcessNotAChild);
    ASSERT_TRUE(pController_->waitForMonitorIteration(1));
    const std::deque<pid_t>& killedProceses = pProcessWrapper_->getProcessedKillCalls();
    //Process for which IProcessWrapper::waitForProcess returned NotAChild status
    //is killed
    ASSERT_THAT(killedProceses, ElementsAre(1));
    
    //New process is generated by the monitor thread.
    processes = pProcessWrapper_->getRunningProcesses();
    ASSERT_EQ(1, processes.size());
    ASSERT_EQ(2, processes[0]);
    
    pController_->stop();
}

TEST_F(TestPmAgentController, CheckFailDuringProcessKill)
{
    ASSERT_EQ(pController_->start(), PM_STATUS::PM_OK);
    ASSERT_TRUE(pController_->waitMonitorThreadInitialized());
    std::vector<pid_t> processes = pProcessWrapper_->getRunningProcesses();
    ASSERT_EQ(1, processes.size());
    ASSERT_EQ(1, processes[0]);
    pProcessWrapper_->addKillCall([](pid_t) {
        //EPERM The calling process does not have permission to send the
        //signal to any of the target processes.
        std::error_code ec = errno_to_error_code(EPERM);
        throw std::system_error(ec);
    });
    ASSERT_EQ(pController_->stop(), PM_STATUS::PM_FAIL);
    
    //Try to force controller to stop monitoring thread
    pController_->setProcessStartedByPlugin(true);
    ASSERT_EQ(pController_->stop(), PM_STATUS::PM_OK);
}

TEST_F(TestPmAgentController, CheckChildForkPath)
{
    pProcessWrapper_->addForkCall([]() -> pid_t {
        //child process
        return 0;
    });
    try
    {
        pController_->start();
        //Real application never exits from the start method in case of child
        //process path. Child process is generated by the fork call and subsequent
        //execv call. This execv call "replaces the current process image with a new process image"
        //which might mean that main function of the new process is called.
        //execv never returns if no errors are occured.
        //For the FakeProcess instance above described behavior is implemented by
        //throwing FakeProcessException exception.
        //So, here we must have FakeProcessException be thrown.
        FAIL();
    }
    catch(FakeProcessException&)
    {
        //do nothing
    }
    catch(...)
    {
        FAIL();
    }
    ASSERT_EQ(1, pProcessWrapper_->getNumberOfExecvCalls());
}

TEST_F(TestPmAgentController, CheckChildForkErrorPath)
{
    pProcessWrapper_->addForkCall([]() -> pid_t {
        //child process
        return 0;
    });
    pProcessWrapper_->addExecvCall([](const std::vector<char *>&) {
        std::error_code ec = errno_to_error_code(E2BIG);
        throw ExecutionError(ec);
    });
    pController_->start();
    ASSERT_EQ(1, pProcessWrapper_->getNumberOfExitCalls());
}
