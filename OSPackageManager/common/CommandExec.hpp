#pragma once

#include <vector>
#include <string>

class  CommandExec
{
public:
    /**
     * @brief Executes a command.
     * @param cmd The command to execute.
     * @param argv The arguments to the command.
     * @param exitCode The exit code of the command.
     * @return 0 if the command was executed successfully.
     */
    static int ExecuteCommand(const std::string cmd, const std::vector<std::string> argv, int &exitCode);

    /**
     * @brief Executes a command and captures the output.
     * @param cmd The command to execute.
     * @param argv The arguments to the command.
     * @param exitCode The exit code of the command.
     * @param output The output of the command.
     * @return 0 if the command was executed successfully.
     */
    static int ExecuteCommandCaptureOutput(const std::string cmd, const std::vector<std::string> argv, int &exitCode, std::string &output);

    /**
     * @brief Parses the output of a command.
     * @param output The output of the command.
     * @param outputLines The output lines of the command.
     * @return void
     */
    static void ParseOutput(const std::string &output, std::vector<std::string> &outputLines);
};