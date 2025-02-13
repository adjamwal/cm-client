#pragma once

#include "IPackageUtil.hpp"
#include "IPmPlatformDiscovery.hpp"

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

private:
    std::shared_ptr<IPackageUtil> pkgUtilManager_; /**< The IPackageUtil instance for package management operations. */
    PackageInventory lastDetectedPackages_;
};

