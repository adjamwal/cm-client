#pragma once
#include <gmock/gmock.h>
#include "IPmPkgUtil.hpp"

class MockPmPkgUtil : public IPmPkgUtil {
public:
    MOCK_METHOD(std::vector<std::string>, listPackages, (const std::string&), (const override));
    MOCK_METHOD(PmPackageInfo, getPackageInfo, (const std::string&, const std::string&), (const override));
    MOCK_METHOD(std::vector<std::string>, listPackageFiles, (const std::string&, const std::string&), (const override));
    MOCK_METHOD(bool, installPackage, (const std::string&, (const std::map<std::string, int>&), const std::string&), (const override));
    MOCK_METHOD(bool, uninstallPackage, (const std::string&), (const override));
    MOCK_METHOD(bool, verifyPackageCodesign, (const std::filesystem::path&, std::string&), (const override));
};
