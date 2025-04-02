#include "PackageUtilDEB.hpp"
#include "PmLogger.hpp"
#include <string.h>

namespace { //anonymous namespace
    const std::string debPackageInstaller {"deb"};
    const std::string dpkgBinStr {"/bin/dpkg"};
    const std::string dpkgGetPkgInfoOption {"-s"};
    const std::string dpkgListPkgFilesOption {"-L"};
    const std::string dpkgInstallPkgOption {"-i"}; //Supports both install and upgrade
    const std::string dpkgUninstallPkgOption {"-P"}; // -P: Purge (Removes configuration files also), -r: Remove (Keeps configuration files)
    const std::string dpkgSigBinStr {"/bin/dpkg-sig"};
    const std::string dpkgSigVerifyOption {"--verify"};
    typedef enum {
        SIG_GOOD = 0,
        SIG_BAD = 1,
        SIG_UNKNOWN = 2,
        SIG_NOT_SIGNED = 3,
        SIG_ERROR = 4
    } SIG_STATUS;

    SIG_STATUS getSigVerificationStatus(const std::string& sigVerifyOutput) {
        if (sigVerifyOutput.find("GOODSIG") == 0) {
            return SIG_GOOD;
        } else if (sigVerifyOutput.find("BADSIG") == 0) {
            return SIG_BAD;
        } else if (sigVerifyOutput.find("UNKNOWNSIG") == 0) {
            return SIG_UNKNOWN;
        } else if (sigVerifyOutput.find("NOSIG") == 0) {
            return SIG_NOT_SIGNED;
        } else {
            return SIG_ERROR;
        }
    };
}

PackageUtilDEB::PackageUtilDEB(ICommandExec &commandExecutor) : commandExecutor_( commandExecutor ) {
}

bool PackageUtilDEB::isValidInstallerType(const std::string &installerType) const {
    return installerType == debPackageInstaller;
}

std::vector<std::string> PackageUtilDEB::listPackages() const {
    // Currently not used, hence not implemented. 
    std::vector<std::string>result;
    return result;
}

PackageInfo PackageUtilDEB::getPackageInfo(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const {
    if(identifierType != PKG_ID_TYPE::NAME) {
        PM_LOG_ERROR("Invalid identifier type. Currently only PKG_NAME is supported.");
        return {};
    }

    PackageInfo packageInfo;
    std::string packageName {};
    std::string packageVersion {};
    std::string packageArchitecture {};
    std::vector<std::string> infoArgv = {dpkgBinStr, dpkgGetPkgInfoOption, packageIdentifier};
    int exitCode = 0;
    std::string outputBuffer;

    int ret = commandExecutor_.ExecuteCommandCaptureOutput(dpkgBinStr, infoArgv, exitCode, outputBuffer);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute get package info command.");
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to get package info. Exit code: %d", exitCode);
    } else {
        std::vector<std::string> outputLines;
        commandExecutor_.ParseOutput(outputBuffer, outputLines);

        for (const auto& line : outputLines) {
            if (line.find("Package:") == 0) {
                packageName = line.substr(strlen("Package: "));
            } else if (line.find("Version:") == 0) {
                packageVersion = line.substr(strlen("Version: "));
            } else if (line.find("Architecture:") == 0) {
                packageArchitecture = line.substr(strlen("Architecture: "));
            }
        }

        // Populate the packageInfo with the obtained values.
        packageInfo.packageIdentifier = packageName + "-" + packageVersion + "." + packageArchitecture;
        packageInfo.packageName = packageName;
        packageInfo.version = packageVersion;
    }

    return packageInfo;
}

std::vector<std::string> PackageUtilDEB::listPackageFiles(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const {
    (void) identifierType; // Currently this is of no use as dpkg -L command works with both name and NVRA format similarly.
    std::vector<std::string>result;
    std::vector<std::string> listArgv = {dpkgBinStr, dpkgListPkgFilesOption, packageIdentifier};
    int exitCode = 0;
    std::string outputBuffer;

    int ret = commandExecutor_.ExecuteCommandCaptureOutput(dpkgBinStr, listArgv, exitCode, outputBuffer);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute list package files command.");
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to list package files. Exit code: %d", exitCode);
    } else {
        commandExecutor_.ParseOutput(outputBuffer, result);
    }  

    return result;
}

bool PackageUtilDEB::installPackage(const std::string& packagePath, const std::map<std::string, int>&  installOptions) const {
    (void) installOptions; // Currently this is of no use.
    std::vector<std::string> installArgv = {dpkgBinStr, dpkgInstallPkgOption, packagePath};
    int exitCode = 0;

    int ret = commandExecutor_.ExecuteCommand(dpkgBinStr, installArgv, exitCode);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute install package command.");
        return false;
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to install package. Exit code: %d", exitCode);
        return false;
    }

    PM_LOG_INFO("Package installed successfully.");
    return true;
}

bool PackageUtilDEB::uninstallPackage(const std::string& packageIdentifier) const {
    std::vector<std::string> uninstallArgv = {dpkgBinStr, dpkgUninstallPkgOption, packageIdentifier};
    int exitCode = 0;

    int ret = commandExecutor_.ExecuteCommand(dpkgBinStr, uninstallArgv, exitCode);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute uninstall package command.");
        return false;
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to uninstall package. Exit code: %d", exitCode);
        return false;
    } 

    PM_LOG_INFO("Package uninstalled successfully.");
    return true;
}

bool PackageUtilDEB::verifyPackage(const std::string& packageIdentifier) const {
    int exit_code = 0;
    std::string sigVerifyOutput;
    std::vector<std::string> sigVerifyLines;
    SIG_STATUS sigStatus = SIG_ERROR;
    std::vector<std::string> package_check_argv{ dpkgSigBinStr, dpkgSigVerifyOption, packageIdentifier };

    int ret = commandExecutor_.ExecuteCommandCaptureOutput(dpkgSigBinStr, package_check_argv, exit_code, sigVerifyOutput);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute verify package command.");
        return false;
    } else { // No need to check exit code as different exit codes represent different signature status which already is taken care by parsing.
        commandExecutor_.ParseOutput(sigVerifyOutput, sigVerifyLines);
    }

    if(sigVerifyLines.size() < 2) {
        PM_LOG_ERROR("Invalid output from dpkg-sig: %s", sigVerifyOutput.c_str());
        return false;
    }

    int lastLine = sigVerifyLines.size() - 1;
    sigStatus = getSigVerificationStatus(sigVerifyLines[lastLine]);
    switch (sigStatus) {
        case SIG_GOOD:
            PM_LOG_INFO("Package %s is signed and verified.", packageIdentifier.c_str());
            return true;
        case SIG_BAD:
            PM_LOG_ERROR("Package %s verification failed due to corrupted signature.", packageIdentifier.c_str());
            return false;
        case SIG_UNKNOWN:
            PM_LOG_ERROR("Package %s verification failed due to unknown signature.", packageIdentifier.c_str());
            return false;
        case SIG_NOT_SIGNED:
            PM_LOG_INFO("Package %s is not signed.", packageIdentifier.c_str());
            return false;
        default:
            PM_LOG_ERROR("Package %s verification failed with unknown error.", packageIdentifier.c_str());
            return false;
    }
}