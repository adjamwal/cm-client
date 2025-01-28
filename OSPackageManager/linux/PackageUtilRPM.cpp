#include <vector>
#include <map>
#include <string>
#include <list>
#include "PmLogger.hpp"
#include "LinuxCommandExec.hpp"
#include "PackageUtilRPM.hpp"

bool PackageUtilRPM::loadLibRPM(void * &libRPMhandle) {

    libRPMhandle = dlmopen(LM_ID_NEWLM, "librpm.so", RTLD_LAZY);
    if (!libRPMhandle) {
        PM_LOG_ERROR("Failed to load librpm: %s", dlerror());
        return false;
    }

    rpmReadConfigFiles_ptr_ = (int (*)(const char*, const char*))dlsym(libRPMhandle, "rpmReadConfigFiles");
    rpmtsCreate_ptr_ = (rpmts (*)(void))dlsym(libRPMhandle, "rpmtsCreate");
    rpmReadPackageFile_ptr_ = (rpmRC (*)(rpmts, FD_t, const char*, Header**))dlsym(libRPMhandle, "rpmReadPackageFile");
    rpmtsFree_ptr_ = (rpmts (*)(rpmts))dlsym(libRPMhandle, "rpmtsFree");
    rpmtsInitIterator_ptr_ = (rpmdbMatchIterator (*)(rpmts, rpmDbiTagVal, const void*, size_t))dlsym(libRPMhandle, "rpmtsInitIterator");
    rpmdbNextIterator_ptr_ = (Header (*)(rpmdbMatchIterator))dlsym(libRPMhandle, "rpmdbNextIterator");
    rpmdbFreeIterator_ptr_ = (rpmdbMatchIterator (*)(rpmdbMatchIterator))dlsym(libRPMhandle, "rpmdbFreeIterator");
    headerGetString_ptr_ = (const char* (*)(Header, rpmTagVal))dlsym(libRPMhandle, "headerGetString");
    if (!rpmReadConfigFiles_ptr_ || !rpmtsCreate_ptr_ || !rpmReadPackageFile_ptr_ || !rpmtsFree_ptr_ || 
        !rpmtsInitIterator_ptr_ || !rpmdbNextIterator_ptr_ || 
        !rpmdbFreeIterator_ptr_ || !headerGetString_ptr_ ) {
        PM_LOG_ERROR("Failed to resolve symbols: %s", dlerror());
        return false;
    }

    rpmReadConfigFiles_ptr_(NULL, NULL);

    return true;
}

bool PackageUtilRPM::unloadLibRPM(void * &libRPMhandle) const {
    int retVal = dlclose(libRPMhandle);
    if(0 != retVal) {
        PM_LOG_ERROR("Failed to unload librpm: %s", dlerror());
        return false;
    }

    libRPMhandle = nullptr;
    return true;
}

std::vector<std::string> PackageUtilRPM::listPackages() const {
    std::vector<std::string>result;
    std::list<std::string>packageList;

    bool status = LinuxCommandExec::execute("rpm -qa", packageList); // packageList would contain all packages in NVRA format as parsed from o/p.
    if (status){
        result.insert(result.end(), packageList.begin(), packageList.end());
    } 
    else {
        PM_LOG_ERROR("Failed to list packages");
    }  

    return result;
}

PackageInfo PackageUtilRPM::getPackageInfo(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const {
    // NOTE: This API assumes the caller is sure that the package exists on the system and is asking for the information.
    //       If the package does not exist, the API will return an empty PackageInfo object.

    PackageInfo result;
    (void) identifierType;
    (void) packageIdentifier;

    void * libRPMhandle = nullptr; // Handle for the load and unload of libRPM library.
    if (!const_cast<PackageUtilRPM*>(this)->loadLibRPM(libRPMhandle)) {
        return result;
    }

    rpmts ts = rpmtsCreate_ptr_();
    rpmdbMatchIterator mi = rpmtsInitIterator_ptr_(ts, RPMDBI_PACKAGES, NULL, 0);

    Header packageHeader;
    while ((packageHeader = rpmdbNextIterator_ptr_(mi)) != NULL) {
        const char* packageName = headerGetString_ptr_(packageHeader, RPMTAG_NAME);
        const char* packageVersion = headerGetString_ptr_(packageHeader, RPMTAG_VERSION);
        const char* packageRelease = headerGetString_ptr_(packageHeader, RPMTAG_RELEASE);
        const char* packageArch = headerGetString_ptr_(packageHeader, RPMTAG_ARCH);
        // NOTE: We are not taking epoch into consideration for now because it will be mostly 0 for the packages we are interested in.
        //       If later there arises a need to consider epoch, we can add RPMTAG_EPOCH to the headerGetString_ptr function and use it here.
        
        if(NULL == packageName || NULL == packageVersion || NULL == packageRelease || NULL == packageArch) {
            PM_LOG_ERROR("Failed to get package info");
            break;
        }

        std::string packageNVRAFormat = std::string(packageName) + \
                                        "-" + std::string(packageVersion) + \
                                        "-" + std::string(packageRelease) + \
                                        "." + std::string(packageArch);

        if(identifierType == PKG_ID_TYPE::NAME && packageName == packageIdentifier) {
            result.packageIdentifier = packageNVRAFormat;
            result.packageName = packageName;
            result.version = packageVersion;
            break;
        } 
        else if(identifierType == PKG_ID_TYPE::NVRA && packageNVRAFormat == packageIdentifier) {
            result.packageIdentifier = packageNVRAFormat;
            result.packageName = packageName;
            result.version = packageVersion;
            break;
        }
    }

    rpmdbFreeIterator_ptr_(mi);
    rpmtsFree_ptr_(ts);

    unloadLibRPM(libRPMhandle);
    return result;
}

std::vector<std::string> PackageUtilRPM::listPackageFiles(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const {
    // Pending Implementation
    std::vector<std::string> result;
    (void) identifierType;
    (void) packageIdentifier;
    return result;
}

bool PackageUtilRPM::installPackage(const std::string& packagePath, const std::map<std::string, int>&  installOptions) const {
    // Pending Implementation
    (void) packagePath;
    (void) installOptions;
    return true;
}

bool PackageUtilRPM::uninstallPackage(const std::string& packageIdentifier) const {
    // Pending Implementation
    (void) packageIdentifier;
    return true;
}