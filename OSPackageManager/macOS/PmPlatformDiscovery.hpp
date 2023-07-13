#pragma once

#include "IPmPkgUtil.hpp"
#include "IPmPlatformDiscovery.hpp"

/**
 * @brief A class that performs platform discovery by utilizing the package manager utility.
 * Implements the IPmPlatformDiscovery interface.
 */
class PmPlatformDiscovery : public IPmPlatformDiscovery {
public:
    /**
     * @brief Constructs a PmPlatformDiscovery object with the specified IPmPkgUtil instance.
     * @param pkgUtil The IPmPkgUtil instance to use for package management operations.
     */
    explicit PmPlatformDiscovery(std::shared_ptr<IPmPkgUtil> pkgUtil) : pkgUtilManager_(pkgUtil) {}
    
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
    std::shared_ptr<IPmPkgUtil> pkgUtilManager_; /**< The IPmPkgUtil instance for package management operations. */
    PackageInventory lastDetectedPackages_;
};

