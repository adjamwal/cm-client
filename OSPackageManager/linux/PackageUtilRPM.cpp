#include "PackageUtilRPM.hpp"
#include "CommandExec.hpp"
#include "PmLogger.hpp"

#define RPM_LIB_PATH "/usr/lib64/librpm.so"
#define RPM_BIN_STR "/bin/rpm"
#define RPM_LIST_PKG_OPTION "-qa"
#define RPM_LIST_PKG_FILES_OPTION "-ql"
#define RPM_INSTALL_PKG_OPTION "-i"
#define RPM_UNINSTALL_PKG_OPTION "-e"

bool PackageUtilRPM::loadLibRPM() {

    libRPMhandle_ = dlmopen(LM_ID_NEWLM, RPM_LIB_PATH, RTLD_LAZY);
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

std::vector<std::string> PackageUtilRPM::listPackages() const {
    std::vector<std::string>result;
    std::string rpmBinString = RPM_BIN_STR;
    std::string listPkgOption = RPM_LIST_PKG_OPTION;
    std::vector<std::string> listArgv = {rpmBinString, listPkgOption};
    int exitCode = 0;
    std::string outputBuffer;

    int ret = CommandExec::ExecuteCommandCaptureOutput(rpmBinString, listArgv, exitCode, outputBuffer);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute list packages command.");
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to list packages. Exit code: %d", exitCode);
    } else {
        CommandExec::ParseOutput(outputBuffer, result);
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
    std::string rpmBinString = RPM_BIN_STR;
    std::string listPkgFilesOption = RPM_LIST_PKG_FILES_OPTION;
    std::vector<std::string> listArgv = {rpmBinString, listPkgFilesOption, packageIdentifier};
    int exitCode = 0;
    std::string outputBuffer;

    int ret = CommandExec::ExecuteCommandCaptureOutput(rpmBinString, listArgv, exitCode, outputBuffer);
    if(ret != 0){
        PM_LOG_ERROR("Failed to execute list package files command.");
    } else if(exitCode != 0) {
        PM_LOG_ERROR("Failed to list package files. Exit code: %d", exitCode);
    } else {
        CommandExec::ParseOutput(outputBuffer, result);
    }  

    return result;
}

bool PackageUtilRPM::installPackage(const std::string& packagePath, const std::map<std::string, int>&  installOptions) const {
    (void) installOptions; // Currently this is of no use.
    std::string rpmBinString = RPM_BIN_STR;
    std::string installPkgOption = RPM_INSTALL_PKG_OPTION;
    std::vector<std::string> installArgv = {rpmBinString, installPkgOption, packagePath};
    int exitCode = 0;

    int ret = CommandExec::ExecuteCommand(rpmBinString, installArgv, exitCode);
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
    std::string rpmBinString = RPM_BIN_STR;
    std::string uninstallPkgOption = RPM_UNINSTALL_PKG_OPTION;
    std::vector<std::string> uninstallArgv = {rpmBinString, uninstallPkgOption, packageIdentifier};
    int exitCode = 0;

    int ret = CommandExec::ExecuteCommand(rpmBinString, uninstallArgv, exitCode);
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