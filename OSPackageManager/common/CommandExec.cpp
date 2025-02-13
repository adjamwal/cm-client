#include "CommandExec.hpp"
#include "PmLogger.hpp"
#include <string>
#include <vector>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <memory>

int CommandExec::ExecuteCommand(const std::string &cmd, const std::vector<std::string> &argv, int &exitCode) {
    int ret = -1;
    int status = 1;
    pid_t pid = -1;
    pid_t waitPid = -1;
    extern char **environ;

    if (cmd.empty() || argv.empty() || '/' != cmd[0]) {
        errno = EINVAL;
        return ret;
    }

    std::vector<char*> argv_cstr;
    for (const auto& arg : argv) {
        argv_cstr.push_back(const_cast<char*>(arg.c_str()));
    }
    argv_cstr.push_back(nullptr);

    if (posix_spawn(&pid, cmd.c_str(), NULL, NULL, argv_cstr.data(), environ) != 0) {
        PM_LOG_ERROR("posix_spawn failed: %s", cmd.c_str());
        return ret;
    }

    PM_LOG_DEBUG("Spawned process for cmd %s: %d", cmd.c_str(), pid);

    while ((waitPid = waitpid(pid, &status, 0)) == -1 && errno == EINTR)
        ;
    if (waitPid == -1) {
        PM_LOG_ERROR("waitpid failed: %d with error code: %d", pid, errno);
        return ret;
    }

    if (WIFEXITED(status)) {
        ret = 0;
        PM_LOG_DEBUG("Process terminated normally: %d (exit code: %d)", pid, WEXITSTATUS(status));
        exitCode = WEXITSTATUS(status);
        return ret;
    }

    errno = ESRCH;

    if (WIFSIGNALED(status)) {
        PM_LOG_ERROR( "Process terminated due to uncaught exception: %d",pid);
    } else if (WIFSTOPPED(status)) {
        PM_LOG_ERROR( "Process stopped abnormally: %d", pid);
    } else {
        PM_LOG_ERROR( "Process did not return: %d", pid);
    }

    return ret;
}

int CommandExec::ExecuteCommandCaptureOutput(const std::string &cmd, const std::vector<std::string> &argv, int &exitCode, std::string &output) {
    int ret = -1;
    int status = 1;
    pid_t pid = -1;
    pid_t waitPid = -1;
    extern char **environ;
    posix_spawn_file_actions_t childFdActions = {0};
    int out [2] = {-1, -1};
    ssize_t bytesRead = 0;
    const int waitPidOptions = 0;   

    if (cmd.empty() || argv.empty() || '/' != cmd[0]) {
        return ret;
    }

    // Lambda Function to delete file actions (Used as custom deleter)
    auto fileActionsDeleter = [](posix_spawn_file_actions_t *actions) {
        posix_spawn_file_actions_destroy(actions);
    };                                                                      

    if (posix_spawn_file_actions_init(&childFdActions) != 0) {
        PM_LOG_ERROR("posix_spawn_file_actions_init failed");
        return ret;
    }

    auto childFdActionsPtr = std::unique_ptr<posix_spawn_file_actions_t, decltype(fileActionsDeleter)>(&childFdActions, fileActionsDeleter); // Just for cleaning purpose on return.

    // Lambda Function to close pipe (Used as custom deleter)
    auto close_pipe = [](int *out) {
        if (out[0] != -1) {
            (void)close(out[0]);
        }
        if (out[1] != -1) {
            (void)close(out[1]);
        }
    };

    auto outPtr = std::unique_ptr<int, decltype(close_pipe)>(out, close_pipe); // Just for cleaning purpose on return.

    if (pipe(out) == -1) {
        PM_LOG_ERROR("pipe failed");
        return ret;
    }

    int flags = fcntl(out[0], F_GETFL, 0); 
    if (flags == -1) {
        PM_LOG_ERROR("fcntl failed");
        return ret;
    }

    if (posix_spawn_file_actions_adddup2(&childFdActions, out[1], 1) != 0) {
        PM_LOG_ERROR("posix_spawn_file_actions_adddup2");
        return ret;
    }

    if (posix_spawn_file_actions_addclose(&childFdActions, out[0]) != 0) {
        PM_LOG_ERROR("posix_spawn_file_actions_addclose");
        return ret;
    }

    std::vector<char*> argv_cstr;
    for (const auto& arg : argv) {
        argv_cstr.push_back(const_cast<char*>(arg.c_str()));
    }
    argv_cstr.push_back(nullptr);

    if (posix_spawn(&pid, cmd.c_str(), &childFdActions, NULL, argv_cstr.data(), environ) != 0) {
        PM_LOG_ERROR("posix_spawn failed: %s", cmd.c_str());
        return ret;
    }

    PM_LOG_DEBUG("Spawned process for cmd %s: %d", cmd.c_str(), pid);

    fcntl(out[0], F_SETFL, flags | O_NONBLOCK);

    do {
        waitPid = waitpid(pid, &status, waitPidOptions);
    } while (waitPid < 0 && errno == EINTR);
    
    if (waitPid < 0) {
        PM_LOG_ERROR("waitpid failed with error: %d", errno);
        return ret;
    } else if (waitPid > 0) {
        if (WIFEXITED(status)) {
            PM_LOG_DEBUG("Process '%s' terminated normally: %d (exit code: %d)", cmd.c_str(), pid, WEXITSTATUS(status));
            
            output.clear();

            char buffer[1024];
            while ((bytesRead = read(out[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytesRead] = '\0';
                output.append(buffer);
            }                       

            exitCode = WEXITSTATUS(status);
            ret = 0;
        } else if (WIFSIGNALED(status)) {
            PM_LOG_ERROR( "Process '%s' terminated due to uncaught exception: %d", cmd.c_str(), pid);
        } else if (WIFSTOPPED(status)) {
            PM_LOG_ERROR( "Process '%s' stopped abnormally: %d", cmd.c_str(), pid);
        } else {
            PM_LOG_ERROR( "Process '%s' did not return: %d", cmd.c_str(), pid);
        }
    }

    return ret;
}

void CommandExec::ParseOutput(const std::string &output, std::vector<std::string> &outputLines) {
    size_t start = 0, end = 0;
    while ((end = output.find('\n', start)) != std::string::npos) {
        outputLines.push_back(output.substr(start, end - start));
        start = end + 1;
    }
    if(start < output.size()) {
        outputLines.push_back(output.substr(start));
    }
}