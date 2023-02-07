#include "gtest/gtest.h"
#include "ModuleControlPlugin.hpp"
#include "CMLogger.hpp"
#include <stddef.h>

#define TEST_LOG_FILEPATH "./ControlPluginTest.log"

namespace {  /* anonymous namespace */

const CMLogger& g_unused_logref(CM_LOG_INIT(TEST_LOG_FILEPATH));

} /* end anonymous namespace */

TEST(ModuleControlPlugin, CreateAndRelease)
{
    PM_MODULE_CTX_T modContext = {0};
    EXPECT_EQ(PM_MODULE_SUCCESS, CreatePMModuleInstance(&modContext));
    EXPECT_NE(nullptr, modContext.fpStart);
    EXPECT_NE(nullptr, modContext.fpStop);
    EXPECT_NE(nullptr, modContext.fpSetOption);

    EXPECT_EQ(nullptr, modContext.fpInit);
    EXPECT_EQ(nullptr, modContext.fpDeinit);
    EXPECT_EQ(nullptr, modContext.fpConfigUpdated);

    EXPECT_EQ(PM_MODULE_NOT_STARTED, modContext.fpStop());
    EXPECT_EQ(PM_MODULE_SUCCESS, modContext.fpStart(".", "", ""));

    //Test, once start has been called on cmpackagemanager process, a
    //further call to it returns 'PM_MODULE_ALREADY_STARTED'
    EXPECT_EQ(PM_MODULE_ALREADY_STARTED, modContext.fpStart(".", "", ""));
    EXPECT_EQ(PM_MODULE_SUCCESS, modContext.fpStop());

    //Test, once stop has been called on cmpackagemanager process, a
    //further call to it returns 'PM_MODULE_NOT_STARTED'
    EXPECT_EQ(PM_MODULE_NOT_STARTED, modContext.fpStop());

    //Test to start cmpackagemanager process after stop has been called on it.
    EXPECT_EQ(PM_MODULE_SUCCESS, modContext.fpStart(".", "", ""));
    EXPECT_EQ(PM_MODULE_SUCCESS, modContext.fpStop());

    EXPECT_EQ(PM_MODULE_SUCCESS, ReleasePMModuleInstance(&modContext));

    //Test if modContext function pointers to start/stop cmpackagemanager
    //process is set to nullptr.
    EXPECT_EQ(nullptr, modContext.fpStart);
    EXPECT_EQ(nullptr, modContext.fpStop);
}
