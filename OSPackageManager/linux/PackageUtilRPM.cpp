#include <cstring>
#include <string.h>
#include <sstream>

#include "PackageUtilRPM.hpp"
#include "PmLogger.hpp"
#include "Gpg/include/GpgKeyId.hpp"

#define LONG_KEY_LEN 16

namespace { //anonymous namespace
    const std::string rpmLibPath {"/usr/lib64/librpm.so"};
    const std::string rpmBinStr {"/bin/rpm"};
    const std::string rpmListPkgOption {"-qa"};
    const std::string rpmListPkgFilesOption {"-ql"};
    const std::string rpmInstallPkgOption {"-U"}; //Supports both install and upgrade
    const std::string rpmUninstallPkgOption {"-e"};
    const std::string rpmKeyFormatStr {"%{RSAHEADER:pgpsig}"};
    const std::string rpmNotSignedStr {"(none)"};
    const std::string rpmKeyIdStr {" Key ID "};
    const std::string rpmPubKeySearchStr {"gpg-pubkey"};
    const std::string rpmPubkeyFormatStr {"%{DESCRIPTION}"};
    const std::string rpmPackageInstaller {"rpm"};
}

PackageUtilRPM::PackageUtilRPM(ICommandExec &commandExecutor, IGpgUtil &gpgUtil) : commandExecutor_( commandExecutor ), gpgUtil_( gpgUtil )  {
    if (!loadLibRPM()) {
        throw PkgUtilException("Failed to load librpm for RPM package operations.");
    }
}

bool PackageUtilRPM::loadLibRPM() {

    libRPMhandle_ = dlopen(rpmLibPath.c_str(), RTLD_NOW);
    if (!libRPMhandle_) {
        PM_LOG_ERROR("Failed to load librpm: %s", dlerror());
        return false;
    }

    fpRpmReadConfigFiles_ = reinterpret_cast<fpRpmReadConfigFiles_t>(dlsym(libRPMhandle_, "rpmReadConfigFiles"));
    fpRpmTsCreate_ = reinterpret_cast<fpRpmTsCreate_t>(dlsym(libRPMhandle_, "rpmtsCreate"));
    fpRpmReadPackageFile_ = reinterpret_cast<fpRpmReadPackageFile_t>(dlsym(libRPMhandle_, "rpmReadPackageFile"));
    fpRpmTsFree_ = reinterpret_cast<fpRpmTsFree_t>(dlsym(libRPMhandle_, "rpmtsFree"));
    fpRpmTsInitIterator_ = reinterpret_cast<fpRpmTsInitIterator_t>(dlsym(libRPMhandle_, "rpmtsInitIterator"));
    fpRpmDbNextIterator_ = reinterpret_cast<fpRpmDbNextIterator_t>(dlsym(libRPMhandle_, "rpmdbNextIterator"));
    fpRpmDbFreeIterator_ = reinterpret_cast<fpRpmDbFreeIterator_t>(dlsym(libRPMhandle_, "rpmdbFreeIterator"));
    fpHeaderGetString_ = reinterpret_cast<fpHeaderGetString_t>(dlsym(libRPMhandle_, "headerGetString"));

    if (!fpRpmReadConfigFiles_ || !fpRpmTsCreate_ || !fpRpmReadPackageFile_ || !fpRpmTsFree_ || 
        !fpRpmTsInitIterator_ || !fpRpmDbNextIterator_ || 
        !fpRpmDbFreeIterator_ || !fpHeaderGetString_ ) {
        PM_LOG_ERROR("Failed to resolve symbols: %s", dlerror());
        unloadLibRPM();
        return false;
    }

    fpRpmReadConfigFiles_(NULL, NULL);

    return true;
}

bool PackageUtilRPM::unloadLibRPM() {
    int retVal = dlclose(libRPMhandle_);
    if(0 != retVal) {
        PM_LOG_ERROR("Failed to unload librpm: %s", dlerror());
        return false;
    }

    libRPMhandle_ = nullptr;
    return true;
}

bool PackageUtilRPM::isValidInstallerType(const std::string &installerType) const {
    return installerType == rpmPackageInstaller;
}

std::vector<std::string> PackageUtilRPM::listPackages() const {
    std::vector<std::string>result;
    std::vector<std::string> listArgv = {rpmBinStr, rpmListPkgOption};
    int exitCode = 0;
    std::string outputBuffer;

    // To-Do: Make sure the rpmBinStr exists before executing the command.
    int ret = commandExecutor_.ExecuteCommandCaptureOutput(rpmBinStr, listArgv, exitCode, outputBuffer);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute list packages command.");
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to list packages. Exit code: %d", exitCode);
    } else {
        commandExecutor_.ParseOutput(outputBuffer, result);
    }

    return result;
}

PackageInfo PackageUtilRPM::getPackageInfo(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const {
    // NOTE: This API assumes the caller is sure that the package exists on the system and is asking for the information.
    //       If the package does not exist, the API will return an empty PackageInfo object.

    PackageInfo result;
    (void) identifierType;
    (void) packageIdentifier;

    rpmts ts = fpRpmTsCreate_();
    if(NULL == ts) {
        PM_LOG_ERROR("Failed to create rpm transaction set.");
        return result;
    }

    rpmdbMatchIterator mi = fpRpmTsInitIterator_(ts, RPMDBI_PACKAGES, NULL, 0);

    Header packageHeader;
    while ((packageHeader = fpRpmDbNextIterator_(mi)) != NULL) {
        const char* packageName = fpHeaderGetString_(packageHeader, RPMTAG_NAME);
        const char* packageVersion = fpHeaderGetString_(packageHeader, RPMTAG_VERSION);
        const char* packageRelease = fpHeaderGetString_(packageHeader, RPMTAG_RELEASE);
        const char* packageArch = fpHeaderGetString_(packageHeader, RPMTAG_ARCH);
        // NOTE: We are not taking epoch into consideration for now because it will be mostly 0 for the packages we are interested in.
        //       If later there arises a need to consider epoch, we can add RPMTAG_EPOCH to the headerGetString_ptr function and use it here.
        
        if(NULL == packageName || NULL == packageVersion || NULL == packageRelease || NULL == packageArch) {
            continue;
        }

        std::string packageNVRAFormat = std::string(packageName) + \
                                        "-" + std::string(packageVersion) + \
                                        "-" + std::string(packageRelease) + \
                                        "." + std::string(packageArch);

        if((identifierType == PKG_ID_TYPE::NAME && packageName == packageIdentifier) || \
            (identifierType == PKG_ID_TYPE::NVRA && packageNVRAFormat == packageIdentifier)) {
            result.packageIdentifier = packageNVRAFormat;
            result.packageName = packageName;
            result.version = packageVersion;
            break;
        }
    }

    fpRpmDbFreeIterator_(mi);
    fpRpmTsFree_(ts);

    return result;
}

std::vector<std::string> PackageUtilRPM::listPackageFiles(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const {
    (void) identifierType; // Currently this is of no use as rpm -ql command works with both name and NVRA format similarly.
    std::vector<std::string>result;
    std::vector<std::string> listArgv = {rpmBinStr, rpmListPkgFilesOption, packageIdentifier};
    int exitCode = 0;
    std::string outputBuffer;

    int ret = commandExecutor_.ExecuteCommandCaptureOutput(rpmBinStr, listArgv, exitCode, outputBuffer);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute list package files command.");
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to list package files. Exit code: %d", exitCode);
    } else {
        commandExecutor_.ParseOutput(outputBuffer, result);
    }  

    return result;
}

bool PackageUtilRPM::installPackage(const std::string& packagePath, const std::map<std::string, int>&  installOptions) const {
    (void) installOptions; // Currently this is of no use.
    std::vector<std::string> installArgv = {rpmBinStr, rpmInstallPkgOption, packagePath};
    int exitCode = 0;

    int ret = commandExecutor_.ExecuteCommand(rpmBinStr, installArgv, exitCode);
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

bool PackageUtilRPM::uninstallPackage(const std::string& packageIdentifier) const {
    std::vector<std::string> uninstallArgv = {rpmBinStr, rpmUninstallPkgOption, packageIdentifier};
    int exitCode = 0;

    int ret = commandExecutor_.ExecuteCommand(rpmBinStr, uninstallArgv, exitCode);
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

bool PackageUtilRPM::is_trusted_by_system(std::string keyId) const {
    std::vector<std::string> imported_rpm_pubkeys_argv = { rpmBinStr, "-q", rpmPubKeySearchStr };
    std::vector<std::string> pubkey_block_argv = { rpmBinStr, "-q", "--queryformat", rpmPubkeyFormatStr, "" };
    const int current_pubkey_index = 4;
    std::vector<std::string> fingerprint_block_argv = { "/bin/gpg", "--fingerprint", keyId };
    std::string gpg_fingerprint = "";
    int exitCode = 0;

    std::string rpm_pubkeys{ "" };
    if (commandExecutor_.ExecuteCommandCaptureOutput(imported_rpm_pubkeys_argv[0],
                                          imported_rpm_pubkeys_argv,
                                          exitCode,
                                          rpm_pubkeys) ||
        (exitCode != 0)) {
        return false;
    }
    std::istringstream rpm_pubkeys_stream;
    rpm_pubkeys_stream.str(rpm_pubkeys);

    for (std::string line; std::getline(rpm_pubkeys_stream, line);) {
        
        std::string pubkey_block = "";
        pubkey_block_argv[current_pubkey_index] = line;
        if (commandExecutor_.ExecuteCommandCaptureOutput(pubkey_block_argv[0],
                                              pubkey_block_argv,
                                              exitCode,
                                              pubkey_block) ||
            (exitCode != 0)) {
            return false;
        }
        
        std::vector<char> pubkey_block_vector(pubkey_block.begin(), pubkey_block.end());
        GpgKeyId trusted_keyid = gpgUtil_.get_pubkey_fingerprint(pubkey_block_vector);
        if (trusted_keyid.long_id() == keyId) {
            return true;
        }
    }

    if (commandExecutor_.ExecuteCommandCaptureOutput(fingerprint_block_argv[0],
                                              fingerprint_block_argv,
                                              exitCode,
                                              gpg_fingerprint) ||
        (exitCode != 0)) {
        return false;
    }

    if (gpg_fingerprint != "gpg: error reading key: No public key") {
        return true;
    }
    return false;
}

static std::string _rpm_get_keyid(std::string pgpsig)
{
    const char *pgpsig_keyid = NULL;

    /* Parse " Key ID <key id>" */
    pgpsig_keyid = std::strstr(pgpsig.c_str(), rpmKeyIdStr.c_str());
    if (!pgpsig_keyid) {
        return "";
    }

    /* Parse "<key id>" */
    pgpsig_keyid += rpmKeyIdStr.length();
    if (*pgpsig_keyid == '\0') {
        return "";
    }

    return std::string(pgpsig_keyid);
}

bool PackageUtilRPM::verifyPackage(const std::string& packageIdentifier) const {
    int exit_code;
    std::string keyId;
    std::string pgpkey;

    std::vector<std::string> package_check_argv{ rpmBinStr, "-q", "--queryformat", rpmKeyFormatStr, "-p", packageIdentifier };

    if (commandExecutor_.ExecuteCommandCaptureOutput(
            rpmBinStr, package_check_argv, exit_code, pgpkey) ||
        (exit_code != 0)) {
        PM_LOG_ERROR("Query package %s for pgpkey failed (%d)", packageIdentifier.c_str(), exit_code);
        return false;
    } else if (rpmNotSignedStr == pgpkey) {
        PM_LOG_ERROR("No key available to validate RPM package - verification failed %s",
            packageIdentifier.c_str());
        return false;
    }

    keyId = _rpm_get_keyid(pgpkey);

    if (keyId.empty()) {
        PM_LOG_INFO("RPM package is not signed: %s", packageIdentifier);
        return false;
    }

    if (is_trusted_by_system(keyId)) {
        return true;
    }
    PM_LOG_INFO("RPM package failed trusted key check: %s", packageIdentifier);
    return false;
}