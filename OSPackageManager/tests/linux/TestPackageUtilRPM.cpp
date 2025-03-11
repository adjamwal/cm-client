/**
* @file
*
* @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
*/

#include "gtest/gtest.h"
#include "Gpg/mock/MockGpgUtil.hpp"
#include "OSPackageManager/Mocks/MockCommandExec/MockCommandExec.hpp"
#include "OSPackageManager/linux/PackageUtilRPM.hpp"
#include "OSPackageManager/common/PmLogger.hpp"

using testing::StrictMock;
using testing::Return;
using testing::_;

class PackageUtilTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      commandExecutorPtr_ = std::make_unique<MockCommandExec>();
      gpgUtilPtr_ = std::make_unique<MockGpgUtil>();
      packageUtil_ = std::make_unique<PackageUtilRPM>(*std::move(commandExecutorPtr_), *std::move(gpgUtilPtr_));
   }
   void TearDown() override
   {
   }
   std::unique_ptr<MockCommandExec> commandExecutorPtr_;
   std::unique_ptr<MockGpgUtil> gpgUtilPtr_;
   std::unique_ptr<PackageUtilRPM> packageUtil_;
};

const int error{ -1 };
const int success{ 0 };
const int error_code_index{ 2 };
const int output_index { 3 };

const std::string keyIdPrefix{ " Key ID " };
const std::string trustedKeyId{ "0123456789abcdef" };
const std::string testKeyId{ keyIdPrefix + trustedKeyId };
const std::string notTrustedKeyId{ "1111111111abcdef" };
const std::string fakePackage{ "package.rpm" };
const std::string pubkeys{ "gpg-pubkey-b86b3716-61e69f29\ngpg-pubkey-c9d8b80b-54c2e3104" };

TEST_F(PackageUtilTest, verifyInvalid)
{  
   auto &gpgUtil{ *gpgUtilPtr_ };
   auto &commandExecutor{ *commandExecutorPtr_ };

   // 1. get pgpkey command fails
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_)).WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(error), Return(success)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsFalse());

   // 2. rpm not signed
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_)).WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>("(none)"), Return(success)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsFalse());

   // 3. cant get key id from string
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_)).WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>("Invalid"), Return(success)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsFalse());

   // 4. " Key ID " is found but actual key is truncated
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_)).WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(keyIdPrefix), Return(success)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsFalse());

   // 5. fail to get rpm pubkeys
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(testKeyId), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(error), ::testing::SetArgReferee<output_index>(""), Return(1)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsFalse());

   // 6. fail to get pubkey block 
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(testKeyId), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(pubkeys), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(error), ::testing::SetArgReferee<output_index>(""), Return(success)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsFalse());
   
   // 7. gpg fingerprint command fails
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(testKeyId), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(""), Return(success))) // empty pubkey block to skip iterating through it
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(error), ::testing::SetArgReferee<output_index>(""), Return(success)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsFalse());

   // 8. gpg fingerprint command succeeds but there is no public key
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(testKeyId), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(""), Return(success))) // empty pubkey block to skip iterating through it
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(error), ::testing::SetArgReferee<output_index>("gpg: error reading key: No public key"), Return(success)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsFalse());

   // 9. first command exec call returns error
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_)).WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), Return(error)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsFalse());

   // 10. second command exec call returns error
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(testKeyId), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(""), Return(error)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsFalse());

   // 11. third command exec call returns error 
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(testKeyId), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(pubkeys), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(""), Return(error)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsFalse());

   // 12. fourth command exec call fails
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(testKeyId), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(""), Return(success))) // empty pubkey block to skip iterating through it (skips 3rd command exec call)
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(""), Return(error)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsFalse());

}

TEST_F(PackageUtilTest, verifyValid)
{
   auto &gpgUtil{ *gpgUtilPtr_ };
   auto &commandExecutor{ *commandExecutorPtr_ };

   //finds matching key id
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(testKeyId), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(pubkeys), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>("Imaginary Pubkey Block"), Return(success)));
   EXPECT_CALL(gpgUtil, get_pubkey_fingerprint(_)).WillOnce(Return(GpgKeyId(trustedKeyId)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsTrue());

   //finds matching key id after iterating through list
   EXPECT_CALL(commandExecutor, ExecuteCommandCaptureOutput(_,_,_,_))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(testKeyId), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>(pubkeys), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>("Imaginary Pubkey Block"), Return(success)))
      .WillOnce(::testing::DoAll(::testing::SetArgReferee<error_code_index>(success), ::testing::SetArgReferee<output_index>("Imaginary Pubkey Block"), Return(success)));
   EXPECT_CALL(gpgUtil, get_pubkey_fingerprint(_))
      .WillOnce(Return(GpgKeyId(notTrustedKeyId)))
      .WillOnce(Return(GpgKeyId(trustedKeyId)));
   ASSERT_THAT(packageUtil_->verifyPackage(fakePackage), ::testing::IsTrue());


}

int main(int argc, char **argv) {
   PmLogger::initLogger();
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
