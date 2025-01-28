#pragma once

#include <string>
#include <list>

class  LinuxCommandExec
{
public:

    /**
     * @brief Executes the supplied command and stores the console output
     * @param rtstrCommand Command to be executed
     * @param rstrResultList List of strings containing parsed console output after the command is executed.
     * @return True if successfully executed, false otherwise. 
     */
    static bool execute(const std::string& rtstrCommand, std::list<std::string>& rstrResultList);

private:

    /**
     * @brief Helper function which executes the supplied command and stores the console output
     * @param rtstrCommand Command to be executed
     * @param rstrResult Console output after the command is executed.
     * @return True if successfully executed, false otherwise.
     */
    static bool executeCmd(const std::string& rtstrCommand, std::string& rstrResult);

    /**
     * @brief Extracts lines from the input string and puts them in a list of strings.
     * @param rstrResult Input string to have lines extracted
     * @param rstrResultList List of lines outputed.
     * @return void
     */
    static void extractLines(const std::string& rstrResult, std::list<std::string>& rstrResultList);
};