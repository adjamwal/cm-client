#include "CommandExec.hpp"
#include "PmLogger.hpp"
#include <string>
#include <vector>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

int CommandExec::ExecuteCommand(char *cmd, char * const *argv, int *exitCode)
{
    int ret = -1;
    int status;
    pid_t pid;
    pid_t waitPid;
    extern char **environ;

    do {
        if (!cmd || !*cmd || !argv || '/' != cmd[0]) {
            errno = EINVAL;
            break;
        }

        if (posix_spawn(&pid, cmd, NULL, NULL, argv, environ) != 0) {
            PM_LOG_ERROR("posix_spawn failed: %s", cmd);
            break;
        }

        PM_LOG_DEBUG("Spawned process for cmd %s: %d", cmd, pid);

        while ((waitPid = waitpid(pid, &status, 0)) == -1 && errno == EINTR)
            ;
        if (waitPid == -1) {
            PM_LOG_ERROR("waitpid failed: %d", pid);
            break;
        }

        if (WIFEXITED(status)) {
            ret = 0;
            PM_LOG_DEBUG("Process terminated normally: %d (exit code: %d)", pid, WEXITSTATUS(status));
            if (exitCode) {
                *exitCode = WEXITSTATUS(status);
            }
            break;
        }

        errno = ESRCH;

        if (WIFSIGNALED(status)) {
            PM_LOG_ERROR( "Process terminated due to uncaught exception: %d",pid);
        } else if (WIFSTOPPED(status)) {
            PM_LOG_ERROR( "Process stopped abnormally: %d", pid);
        } else {
            PM_LOG_ERROR( "Process did not return: %d", pid);
        }
    } while (0);

    return ret;
}

int CommandExec::ExecuteCommandCaptureOutput(char *cmd, char * const *argv, int *exitCode, std::string &output)
{
    int ret = -1;
    int status;
    pid_t pid;
    pid_t waitPid;
    extern char **environ;
    posix_spawn_file_actions_t childFdActions;
    bool childFdActions_init = false;
    int out[2] = { -1, -1 };
    ssize_t bytesRead = 0;
    fd_set readFds;
    const int waitPidOptions = 0;
    const struct timespec selectTimeout = { .tv_nsec = 10000000 };
    const struct timespec cmdPollInterval = { .tv_nsec = 10000000 };
    struct timespec cmdTimeRemaining = { .tv_sec = 0 };
    int flags = fcntl(out[0], F_GETFL, 0);
    

    if (!cmd || !*cmd || !argv || '/' != cmd[0]) {
        goto done;
    }

    if (posix_spawn_file_actions_init(&childFdActions) != 0) {
        PM_LOG_ERROR("posix_spawn_file_actions_init failed");
        goto done;
    }
    childFdActions_init = true;

    if (pipe(out) == -1) {
        PM_LOG_ERROR("pipe failed");
        goto done;
    }

    if (posix_spawn_file_actions_adddup2(&childFdActions, out[1], 1) != 0) {
        PM_LOG_ERROR("posix_spawn_file_actions_adddup2");
        goto done;
    }

    if (posix_spawn_file_actions_addclose(&childFdActions, out[0]) != 0) {
        PM_LOG_ERROR("posix_spawn_file_actions_addclose");
        goto done;
    }

    if (posix_spawn(&pid, cmd, &childFdActions, NULL, argv, environ) != 0) {
        PM_LOG_ERROR("posix_spawn failed: %s", cmd);
        goto done;
    }

    PM_LOG_DEBUG("Spawned process for cmd %s: %d", cmd, pid);

    fcntl(out[0], F_SETFL, flags | O_NONBLOCK);

    for (;;) {
        waitPid = waitpid(pid, &status, waitPidOptions);
        if (waitPid < 0) {
            if (errno == EINTR) {
                continue;
            }
            PM_LOG_ERROR("waitpid failed");
            break;
        } else if (waitPid > 0) {
            if (WIFEXITED(status)) {
                int err;

                PM_LOG_DEBUG("Process '%s' terminated normally: %d (exit code: %d)", cmd, pid, WEXITSTATUS(status));

                FD_ZERO(&readFds);
                FD_SET(out[0], &readFds);
                output.clear();

                while ((err = pselect(out[0] + 1, &readFds, NULL, NULL, &selectTimeout, NULL)) < 0 &&
                       errno == EINTR)
                    ;
                if (err < 0) {
                    PM_LOG_ERROR("pselect failed");
                    goto done;
                } else if (err == 1) {
                    char buffer[1024];
                    while ((bytesRead = read(out[0], buffer, sizeof(buffer) - 1)) > 0) {
                        buffer[bytesRead] = '\0';
                        output.append(buffer);                       
                    }
                }

                if (exitCode) {
                    *exitCode = WEXITSTATUS(status);
                }
                ret = 0;
            } else if (WIFSIGNALED(status)) {
                PM_LOG_ERROR( "Process '%s' terminated due to uncaught exception: %d", cmd, pid);
            } else if (WIFSTOPPED(status)) {
                PM_LOG_ERROR( "Process '%s' stopped abnormally: %d", cmd, pid);
            } else {
                PM_LOG_ERROR( "Process '%s' did not return: %d", cmd, pid);
            }
            break;
        }
        nanosleep(&cmdPollInterval, NULL);
        cmdTimeRemaining.tv_sec -= cmdPollInterval.tv_sec;
        cmdTimeRemaining.tv_nsec -= cmdPollInterval.tv_nsec;
        if (cmdTimeRemaining.tv_sec < 0 || cmdTimeRemaining.tv_nsec < 0) {
            PM_LOG_ERROR( "Process '%s' timed out, killing: %d", cmd, pid);
            if (!kill(pid, SIGKILL)) {
                (void)waitpid(pid, &status, 0);
            }
            break;
        }
    }
done:
    if (out[0] != -1) {
        (void)close(out[0]);
    }
    if (out[1] != -1) {
        (void)close(out[1]);
    }
    if (childFdActions_init) {
        posix_spawn_file_actions_destroy(&childFdActions);
    }

    return ret;
}

void CommandExec::ParseOutput(const std::string &output, std::vector<std::string> &outputLines)
{
    size_t start = 0, end = 0;
    while ((end = output.find('\n', start)) != std::string::npos) {
        outputLines.push_back(output.substr(start, end - start));
        start = end + 1;
    }
    if(start < output.size()) {
        outputLines.push_back(output.substr(start));
    }
}