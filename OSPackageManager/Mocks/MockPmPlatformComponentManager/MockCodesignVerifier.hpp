#pragma once

#include "IPmCodesignVerifier.hpp"
#include "gmock/gmock.h"

// Mock for CodesignVerifier
class MockCodesignVerifier : public IPmCodesignVerifier {
public:
    MOCK_METHOD(CodeSignStatus, ExecutableVerify, ( const std::filesystem::path&, const std::string&, SigType), (override));
    MOCK_METHOD(CodeSignStatus, ExecutableVerifyWithKilldate, (const std::filesystem::path&, const std::string&, SigType, uint64_t), (override));
    MOCK_METHOD(CodeSignStatus, PackageVerify, (const std::filesystem::path&, const std::string&), (override));
};
