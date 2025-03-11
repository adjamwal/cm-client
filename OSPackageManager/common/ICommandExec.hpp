#pragma once

#include <vector>
#include <string>

class  ICommandExec
{
public:
    ICommandExec() = default;
    virtual ~ICommandExec() = default;
    /**
     * @brief Executes a command.
     * @param cmd The command to execute.
     * @param argv The arguments to the command.
     * @param exitCode The exit code of the command.
     * @return 0 if the command was executed successfully.
     */
    virtual int ExecuteCommand(const std::string &cmd, const std::vector<std::string> &argv, int &exitCode) = 0;

    /**
     * @brief Executes a command and captures the output.
     * @param cmd The command to execute.
     * @param argv The arguments to the command.
     * @param exitCode The exit code of the command.
     * @param output The output of the command.
     * @return 0 if the command was executed successfully.
     */
    virtual int ExecuteCommandCaptureOutput(const std::string &cmd, const std::vector<std::string> &argv, int &exitCode, std::string &output) = 0;

    /**
     * @brief Parses the output of a command.
     * @param output The output of the command.
     * @param outputLines The output lines of the command.
     * @return void
     */
    virtual void ParseOutput(const std::string &output, std::vector<std::string> &outputLines) = 0;
};