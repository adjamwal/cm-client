/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once
#include "MockEnvironment.h"
#include "gtest/gtest.h"


namespace TestEnv
{
class UnitTestBase: public ::testing::Test
{
protected:
    MockEnvironment mockEnv_;
};
}
