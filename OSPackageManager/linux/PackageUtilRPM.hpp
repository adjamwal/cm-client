#pragma once

#include "IPackageUtil.hpp"

/**
 * @brief A class that implements the 'PackageUtil' utility to perform package-related operations for RPM.
 * Implements the IPackageUtil interface.
 */
class PackageUtilRPM : public IPackageUtil {
public:

    /**
     * @brief Delay loads the libRPM library.
     * @param libRPMhandle Reference of handle for libRPM library.
     * @return True if successfully loaded, false otherwise. 
     */
    bool loadLibRPM(void * &libRPMhandle) const;

    /**
     * @brief Unloads the libRPM library.
     * @param libRPMhandle Reference of handle for libRPM library.
     * @return True if successfully unloaded, false otherwise. 
     */
    bool unloadLibRPM(void * &libRPMhandle) const;
    
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
};
