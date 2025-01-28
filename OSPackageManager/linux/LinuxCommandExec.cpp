#include "LinuxCommandExec.hpp"

#define COMMAND_SHELL_BUFFER_SIZE     1024

bool LinuxCommandExec::executeCmd(const std::string& rtstrCommand, std::string& rstrResult)
{
    char   szCmdBuffer[COMMAND_SHELL_BUFFER_SIZE] = {0};
    FILE *pCmdOutput = popen(rtstrCommand.c_str(), "r");

    if(NULL == pCmdOutput)
    {
        return false;
    }
    while(fgets(szCmdBuffer, COMMAND_SHELL_BUFFER_SIZE, pCmdOutput))
    {
        rstrResult += szCmdBuffer;
    }
    pclose( pCmdOutput );
    return true;
}

void LinuxCommandExec::extractLines(const std::string& rstrResult,
    std::list<std::string>& rstrResultList)
{
    unsigned int begin = 0, end = 0;
    end = rstrResult.find('\n', begin);
    while (end < static_cast<unsigned int>(rstrResult.size()))
    {
        // get the new line and remove carriage return if it exist
        std::string::size_type pos;
        std::string newLine(rstrResult, begin, end - begin);
        pos = newLine.find('\r', 0);
        if (pos != std::string::npos)
        {
            newLine.erase(pos, newLine.length() - pos);
        }

        rstrResultList.push_back(newLine);
        begin = end + 1;
        end = rstrResult.find('\n', begin);
    }
}

bool LinuxCommandExec::execute(const std::string& rCommand,
    std::list<std::string>& rstrResultList)
{
    bool status = true;
    if (rCommand.empty())
    {
        status = false;
    }
    else
    {
        std::string result;
        status = executeCmd(rCommand, result);
        if (status)
        {
            extractLines(result, rstrResultList);
        }
    }

    return status;
}