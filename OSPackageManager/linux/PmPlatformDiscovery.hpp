#pragma once

#include "IPackageUtil.hpp"
#include "IPmPlatformDiscovery.hpp"
#include "IFileUtilities.hpp"

/**
 * @brief A class that performs platform discovery by utilizing the package manager utility.
 * Implements the IPmPlatformDiscovery interface.
 */
class PmPlatformDiscovery : public IPmPlatformDiscovery {
public:
    /**
     * @brief Constructs a PmPlatformDiscovery object with the specified IPackageUtil instance.
     * @param pkgUtil The IPackageUtil instance to use for package management operations.
     * @param fileUtils The IFileUtilities instance to use for file operations. 
     */
    PmPlatformDiscovery(std::shared_ptr<IPmPkgUtil> pkgUtil) : pkgUtilManager_(std::move(pkgUtil)) {}
    
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
    void ProcessPackageDiscovery(
        const PmProductDiscoveryRules& rule,
        const std::string& pkgIdentifier,
        const std::vector<std::string>& packageList,
        PKG_ID_TYPE pkgType,
        std::set<std::string>& uniquePks,
        PackageInventory& packagesDiscovered);
    std::shared_ptr<IPackageUtil> pkgUtilManager_; /**< The IPackageUtil instance for package management operations. */
    std::shared_ptr<PackageManager::IFileUtilities> fileUtils_;
    PackageInventory lastDetectedPackages_;
};

