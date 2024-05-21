#pragma once

#include "IPmPkgUtil.hpp"
#include "IPmPlatformDiscovery.hpp"
#include "IFileUtilities.hpp"

/**
 * @brief A class that performs platform discovery by utilizing the package manager utility.
 * Implements the IPmPlatformDiscovery interface.
 */
class PmPlatformDiscovery : public IPmPlatformDiscovery {
public:
    /**
     * @brief Constructs a PmPlatformDiscovery object with the specified IPmPkgUtil instance.
     * @param pkgUtil The IPmPkgUtil instance to use for package management operations.
     * @param fileUtils The IFileUtilities instance to use for file operations. 
     */
    PmPlatformDiscovery(std::shared_ptr<IPmPkgUtil> pkgUtil, std::shared_ptr<PackageManager::IFileUtilities> fileUtils) : pkgUtilManager_(std::move(pkgUtil)), fileUtils_(std::move(fileUtils)) {}
    
    /**
     * @brief Discovers the installed packages based on the provided catalog rules.
     * @param catalogRules The catalog rules to use for package discovery.
     * @return The inventory of discovered packages.
     */
    PackageInventory DiscoverInstalledPackages(const std::vector<PmProductDiscoveryRules>& catalogRules) override;
    
    /**
     * @brief Returns installed packages inventory from a cache
     * @return The inventory of discovered packages.
     */
    PackageInventory CachedInventory() const override;
    
protected:
    void ResolveAndDiscover(
        const std::filesystem::path& unresolvedPath,
        const std::filesystem::path& resolvedPath,
        std::string& out_knownFolderId,
        std::string& out_knownFolderIdConversion,
        std::vector<std::filesystem::path>& out_discoveredFiles );

    void DiscoverPackageConfigurables(
        const std::vector<PmProductDiscoveryConfigurable>& configurables,
        std::vector<PackageConfigInfo>& packageConfigs );

private:
    std::shared_ptr<IPmPkgUtil> pkgUtilManager_; /**< The IPmPkgUtil instance for package management operations. */
    std::shared_ptr<PackageManager::IFileUtilities> fileUtils_;
    PackageInventory lastDetectedPackages_;
};

