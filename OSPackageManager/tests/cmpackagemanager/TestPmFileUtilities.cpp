/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "gtest/gtest.h"
#include "PmPlatformComponentManager.hpp"
#include "MockPmPlatformComponentManager/MockCodesignVerifier.hpp"

#include "UnitTestBase.h"

using testing::StrictMock;
using testing::Return;
using testing::_;

class TestPmFileUtilities: public TestEnv::UnitTestBase{
public:
    TestPmFileUtilities()
    : compManager_(mockEnv_.pkgUtil_, std::make_shared<StrictMock<MockCodesignVerifier>>(), mockEnv_.fileUtils_)
    {}

    ~TestPmFileUtilities()
    {}

protected:
    PmPlatformComponentManager compManager_;
};

TEST_F(TestPmFileUtilities, TestInvalidPath)
{
    EXPECT_CALL(*mockEnv_.fileUtils_, HasAdminRestrictionsApplied(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, HasUserRestrictionsApplied(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, ApplyAdminRestrictions(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, ApplyUserRestrictions(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, PathIsValid(_)).Times(1);
    ON_CALL( *mockEnv_.fileUtils_, PathIsValid(_) ).WillByDefault( Return( false ) );
    EXPECT_THAT(compManager_.RestrictPathPermissionsToAdmins({}), -1);
}

TEST_F(TestPmFileUtilities, TestHasAdminRestrictionsAlready)
{
    EXPECT_CALL(*mockEnv_.fileUtils_, HasAdminRestrictionsApplied(_)).Times(1);
    EXPECT_CALL(*mockEnv_.fileUtils_, HasUserRestrictionsApplied(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, ApplyAdminRestrictions(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, ApplyUserRestrictions(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, PathIsValid(_)).Times(1);
    ON_CALL( *mockEnv_.fileUtils_, PathIsValid(_) ).WillByDefault( Return( true ) );
    ON_CALL( *mockEnv_.fileUtils_, HasAdminRestrictionsApplied(_) ).WillByDefault( Return( true ) );
    EXPECT_THAT(compManager_.RestrictPathPermissionsToAdmins({}), 0);
}

TEST_F(TestPmFileUtilities, TestHasNoAdminRestrictionsAlready)
{
    EXPECT_CALL(*mockEnv_.fileUtils_, HasAdminRestrictionsApplied(_)).Times(1);
    EXPECT_CALL(*mockEnv_.fileUtils_, HasUserRestrictionsApplied(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, ApplyAdminRestrictions(_)).Times(1);
    EXPECT_CALL(*mockEnv_.fileUtils_, ApplyUserRestrictions(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, PathIsValid(_)).Times(1);
    ON_CALL( *mockEnv_.fileUtils_, PathIsValid(_) ).WillByDefault( Return( true ) );
    ON_CALL( *mockEnv_.fileUtils_, HasAdminRestrictionsApplied(_) ).WillByDefault( Return( false ) );
    ON_CALL( *mockEnv_.fileUtils_, ApplyAdminRestrictions(_) ).WillByDefault( Return( true ) );
    EXPECT_THAT(compManager_.RestrictPathPermissionsToAdmins({}), 0);
}

TEST_F(TestPmFileUtilities, TestFailAdminRestrictionsApply)
{
    EXPECT_CALL(*mockEnv_.fileUtils_, HasAdminRestrictionsApplied(_)).Times(1);
    EXPECT_CALL(*mockEnv_.fileUtils_, HasUserRestrictionsApplied(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, ApplyAdminRestrictions(_)).Times(1);
    EXPECT_CALL(*mockEnv_.fileUtils_, ApplyUserRestrictions(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, PathIsValid(_)).Times(1);
    ON_CALL( *mockEnv_.fileUtils_, PathIsValid(_) ).WillByDefault( Return( true ) );
    ON_CALL( *mockEnv_.fileUtils_, HasAdminRestrictionsApplied(_) ).WillByDefault( Return( false ) );
    ON_CALL( *mockEnv_.fileUtils_, ApplyAdminRestrictions(_) ).WillByDefault( Return( false ) );
    EXPECT_THAT(compManager_.RestrictPathPermissionsToAdmins({}), -1);
}

TEST_F(TestPmFileUtilities, TestApplyBultinUsersReadPermissionsInvalidPath)
{
    EXPECT_CALL(*mockEnv_.fileUtils_, HasUserRestrictionsApplied(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, ApplyAdminRestrictions(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, ApplyUserRestrictions(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, PathIsValid(_)).Times(1);
    ON_CALL( *mockEnv_.fileUtils_, PathIsValid(_) ).WillByDefault( Return( false ) );
    EXPECT_THAT(compManager_.ApplyBultinUsersReadPermissions({}), -1);
}

TEST_F(TestPmFileUtilities, TestHasUserRestrictionsAlready)
{
    EXPECT_CALL(*mockEnv_.fileUtils_, HasUserRestrictionsApplied(_)).Times(1);
    EXPECT_CALL(*mockEnv_.fileUtils_, ApplyUserRestrictions(_)).Times(0);
    EXPECT_CALL(*mockEnv_.fileUtils_, PathIsValid(_)).Times(1);
    ON_CALL(*mockEnv_.fileUtils_, PathIsValid(_)).WillByDefault(Return(true));
    ON_CALL(*mockEnv_.fileUtils_, HasUserRestrictionsApplied(_)).WillByDefault(Return(true));
    EXPECT_THAT(compManager_.ApplyBultinUsersReadPermissions({}), 0);
}

TEST_F(TestPmFileUtilities, TestApplyUserRestrictions_Success)
{
    EXPECT_CALL(*mockEnv_.fileUtils_, HasUserRestrictionsApplied(_)).Times(1);
    EXPECT_CALL(*mockEnv_.fileUtils_, ApplyUserRestrictions(_)).Times(1);
    EXPECT_CALL(*mockEnv_.fileUtils_, PathIsValid(_)).Times(1);
    ON_CALL(*mockEnv_.fileUtils_, PathIsValid(_)).WillByDefault(Return(true));
    ON_CALL(*mockEnv_.fileUtils_, HasUserRestrictionsApplied(_)).WillByDefault(Return(false));
    ON_CALL(*mockEnv_.fileUtils_, ApplyUserRestrictions(_)).WillByDefault(Return(true));
    EXPECT_THAT(compManager_.ApplyBultinUsersReadPermissions({}), 0);
}

TEST_F(TestPmFileUtilities, TestApplyUserRestrictions_Failed)
{
    EXPECT_CALL(*mockEnv_.fileUtils_, HasUserRestrictionsApplied(_)).Times(1);
    EXPECT_CALL(*mockEnv_.fileUtils_, ApplyUserRestrictions(_)).Times(1);
    EXPECT_CALL(*mockEnv_.fileUtils_, PathIsValid(_)).Times(1);
    ON_CALL(*mockEnv_.fileUtils_, PathIsValid(_)).WillByDefault(Return(true));
    ON_CALL(*mockEnv_.fileUtils_, HasUserRestrictionsApplied(_)).WillByDefault(Return(false));
    ON_CALL(*mockEnv_.fileUtils_, ApplyUserRestrictions(_)).WillByDefault(Return(false));
    EXPECT_THAT(compManager_.ApplyBultinUsersReadPermissions({}), -1);
}
