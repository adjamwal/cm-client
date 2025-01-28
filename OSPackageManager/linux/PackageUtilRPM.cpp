#include <vector>
#include <map>
#include <string>
#include <list>
#include <dlfcn.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>
#include "PmLogger.hpp"
#include "PackageUtilRPM.hpp"

#define COMMAND_SHELL_BUFFER_SIZE     1024

// Function pointers for librpm functions
int (*rpmReadConfigFiles_ptr)(const char*, const char*);
rpmts (*rpmtsCreate_ptr)(void);
rpmRC (*rpmReadPackageFile_ptr)(rpmts, FD_t, const char*, Header**);
rpmts (*rpmtsFree_ptr)(rpmts);
rpmdbMatchIterator (*rpmtsInitIterator_ptr)(rpmts, rpmDbiTagVal, const void*, size_t);
Header (*rpmdbNextIterator_ptr)(rpmdbMatchIterator);
rpmdbMatchIterator (*rpmdbFreeIterator_ptr)(rpmdbMatchIterator);
const char* (*headerGetString_ptr)(Header, rpmTagVal);


bool PackageUtilRPM::loadLibRPM(void * &libRPMhandle_) const {

    libRPMhandle_ = dlmopen(LM_ID_NEWLM, "librpm.so", RTLD_LAZY);
    if (!libRPMhandle_) {
        PM_LOG_ERROR("Failed to load librpm: %s", dlerror());
        return false;
    }

    rpmReadConfigFiles_ptr = (int (*)(const char*, const char*))dlsym(libRPMhandle_, "rpmReadConfigFiles");
    rpmtsCreate_ptr = (rpmts (*)(void))dlsym(libRPMhandle_, "rpmtsCreate");
    rpmReadPackageFile_ptr = (rpmRC (*)(rpmts, FD_t, const char*, Header**))dlsym(libRPMhandle_, "rpmReadPackageFile");
    rpmtsFree_ptr = (rpmts (*)(rpmts))dlsym(libRPMhandle_, "rpmtsFree");
    rpmtsInitIterator_ptr = (rpmdbMatchIterator (*)(rpmts, rpmDbiTagVal, const void*, size_t))dlsym(libRPMhandle_, "rpmtsInitIterator");
    rpmdbNextIterator_ptr = (Header (*)(rpmdbMatchIterator))dlsym(libRPMhandle_, "rpmdbNextIterator");
    rpmdbFreeIterator_ptr = (rpmdbMatchIterator (*)(rpmdbMatchIterator))dlsym(libRPMhandle_, "rpmdbFreeIterator");
    headerGetString_ptr = (const char* (*)(Header, rpmTagVal))dlsym(libRPMhandle_, "headerGetString");
    if (!rpmReadConfigFiles_ptr || !rpmtsCreate_ptr || !rpmReadPackageFile_ptr || !rpmtsFree_ptr || 
        !rpmtsInitIterator_ptr || !rpmdbNextIterator_ptr || 
        !rpmdbFreeIterator_ptr || !headerGetString_ptr ) {
        PM_LOG_ERROR("Failed to resolve symbols: %s", dlerror());
        return false;
    }

    rpmReadConfigFiles_ptr(NULL, NULL);

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

bool executeCmd(const std::string& rtstrCommand, std::string& rstrResult)
{
    char   szCmdBuffer[COMMAND_SHELL_BUFFER_SIZE] = {0};
    FILE *pCmdOutput = popen(rtstrCommand.c_str(), "r");

    if(NULL == pCmdOutput)
    {
        return false;
    }
    while(fgets(szCmdBuffer, COMMAND_SHELL_BUFFER_SIZE, pCmdOutput))
    {
        rstrResult += szCmdBuffer;
    }
    pclose( pCmdOutput );
    return true;
}

void extractLines(const std::string& rstrResult,
    std::list<std::string>& rstrResultList)
{
    unsigned int begin = 0, end = 0;
    end = rstrResult.find('\n', begin);
    while (end < static_cast<unsigned int>(rstrResult.size()))
    {
        // get the new line and remove carriage return if it exist
        std::string::size_type pos;
        std::string newLine(rstrResult, begin, end - begin);
        pos = newLine.find('\r', 0);
        if (pos != std::string::npos)
        {
            newLine.erase(pos, newLine.length() - pos);
        }

        rstrResultList.push_back(newLine);
        begin = end + 1;
        end = rstrResult.find('\n', begin);
    }
}

bool execute(const std::string& rCommand,
    std::list<std::string>& rstrResultList)
{
    bool status = true;
    if (rCommand.empty())
    {
        status = false;
    }
    else
    {
        std::string result;
        status = executeCmd(rCommand, result);
        if (status)
        {
            extractLines(result, rstrResultList);
        }
    }

    return status;
}

std::vector<std::string> PackageUtilRPM::listPackages() const {
    std::vector<std::string>result;
    std::list<std::string>packageList;

    bool status = execute("rpm -qa", packageList); // packageList would contain all packages in NVRA format as parsed from o/p.
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
    if (!loadLibRPM(libRPMhandle)) {
        return result;
    }

    rpmts ts = rpmtsCreate_ptr();
    rpmdbMatchIterator mi = rpmtsInitIterator_ptr(ts, RPMDBI_PACKAGES, NULL, 0);

    Header packageHeader;
    while ((packageHeader = rpmdbNextIterator_ptr(mi)) != NULL) {
        const char* packageName = headerGetString_ptr(packageHeader, RPMTAG_NAME);
        const char* packageVersion = headerGetString_ptr(packageHeader, RPMTAG_VERSION);
        const char* packageRelease = headerGetString_ptr(packageHeader, RPMTAG_RELEASE);
        const char* packageArch = headerGetString_ptr(packageHeader, RPMTAG_ARCH);
        // NOTE: We are not taking epoch into consideration for now because it will be mostly 0 for the packages we are interested in.
        //       If later there arises a need to consider epoch, we can add RPMTAG_EPOCH to the headerGetString_ptr function and use it here.

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

    rpmdbFreeIterator_ptr(mi);
    rpmtsFree_ptr(ts);

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