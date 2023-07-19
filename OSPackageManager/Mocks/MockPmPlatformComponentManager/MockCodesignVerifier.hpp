#pragma once

#include "IPmCodesignVerifier.hpp"
#include "gmock/gmock.h"

// Mock for CodesignVerifier
class MockCodesignVerifier : public IPmCodesignVerifier {
public:
    MOCK_METHOD(CodeSignStatus, Verify, ( const std::string&, const std::string&, SigType), (override));
    MOCK_METHOD(CodeSignStatus, VerifyWithKilldate, (const std::string&, const std::string&, SigType, uint64_t), (override));
};
