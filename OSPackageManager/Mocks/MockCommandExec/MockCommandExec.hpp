/**
 * @file
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved.
 */
#pragma once

#include "OSPackageManager/common/ICommandExec.hpp"
#include "gmock/gmock.h"

class MockCommandExec : public ICommandExec
{
    public:
        MOCK_METHOD(int, ExecuteCommand, (const std::string &cmd, const std::vector<std::string> &argv, int &exitCode), (override));
        MOCK_METHOD(int, ExecuteCommandCaptureOutput, (const std::string &cmd, const std::vector<std::string> &argv, int &exitCode, std::string &output), (override));
        MOCK_METHOD(void, ParseOutput, (const std::string &output, std::vector<std::string> &outputLines), (override));
};
