#pragma once

#include "IPackageUtil.hpp"
#include <dlfcn.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmts.h>
#include <rpm/rpmdb.h>

typedef int (*fpRpmReadConfigFiles_t)(const char*, const char*);
typedef rpmts (*fpRpmTsCreate_t)(void);
typedef rpmRC (*fpRpmReadPackageFile_t)(rpmts, FD_t, const char*, Header**);
typedef rpmts (*fpRpmTsFree_t)(rpmts);
typedef rpmdbMatchIterator (*fpRpmTsInitIterator_t)(rpmts, rpmDbiTagVal, const void*, size_t);
typedef Header (*fpRpmDbNextIterator_t)(rpmdbMatchIterator);
typedef rpmdbMatchIterator (*fpRpmDbFreeIterator_t)(rpmdbMatchIterator);
typedef const char* (*fpHeaderGetString_t)(Header, rpmTagVal);

/**
 * @brief A class that implements the 'PackageUtil' utility to perform package-related operations for RPM.
 * Implements the IPackageUtil interface.
 */
class PackageUtilRPM : public IPackageUtil {
public:

    /**
     * @brief Constructor to load librpm for RPM package operations.
     */
    PackageUtilRPM() {
        if(!loadLibRPM()) {
            throw PackageUtilException("Failed to load librpm for RPM package operations.");
        }
    }

    /**
     * @brief Destructor to unload librpm for RPM package operations.
     */
    ~PackageUtilRPM() {
        unloadLibRPM();
    }

    /**
     * @brief Lists the packages installed.
     * @return A vector of package identifiers.
     */
    std::vector<std::string> listPackages() const override;
    
    /**
     * @brief Retrieves information about a specific package.
     * @param identifierType Type of Package identifier.
     * @param packageIdentifier The identifier of the package.
     * @return The package information.
     * @note   This API assumes the caller is sure that the package exists on the system and is asking for the information.
     *         If the package does not exist, the API will return an empty PackageInfo object.
     */
    PackageInfo getPackageInfo(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const override;
    
    /**
     * @brief Lists the files contained within a specific package.
     * @param identifierType Type of Package identifier.
     * @param packageIdentifier The identifier of the package.
     * @return A vector of Package Files.
     */
    std::vector<std::string> listPackageFiles(const PKG_ID_TYPE& identifierType, const std::string& packageIdentifier) const override;
    
    /**
     * @brief Installs a package from the specified path onto the specified volume or the default volume.
     * @param packagePath The path to the package.
     * @param installOptions Options for installation (optional).
     * @return True if the installation was successful, false otherwise.
     */
    bool installPackage(const std::string& packagePath, const std::map<std::string, int>&  installOptions = {}) const override;
    
    /**
     * @brief Uninstalls a package with the specified identifier.
     * @param packageIdentifier The identifier of the package.
     * @return True if the uninstallation was successful, false otherwise.
     */
    bool uninstallPackage(const std::string& packageIdentifier) const override;

private:
    void* libRPMhandle_ = nullptr; // Handle for the load and unload of libRPM library.

    // Function pointers for librpm functions
    fpRpmReadConfigFiles_t fpRpmReadConfigFiles_ = nullptr;
    fpRpmTsCreate_t fpRpmTsCreate_ = nullptr;
    fpRpmReadPackageFile_t fpRpmReadPackageFile_ = nullptr;
    fpRpmTsFree_t fpRpmTsFree_ = nullptr;
    fpRpmTsInitIterator_t fpRpmTsInitIterator_ = nullptr;
    fpRpmDbNextIterator_t fpRpmDbNextIterator_ = nullptr;
    fpRpmDbFreeIterator_t fpRpmDbFreeIterator_ = nullptr;
    fpHeaderGetString_t fpHeaderGetString_ = nullptr;

    /**
     * @brief Delay loads the libRPM library.
     * @return True if successfully loaded, false otherwise. 
     */
    bool loadLibRPM();

    /**
     * @brief Unloads the libRPM library.
     * @return True if successfully unloaded, false otherwise. 
     */
    bool unloadLibRPM();
};
