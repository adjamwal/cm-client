#include "PmPlatformDiscovery.hpp"

PackageInventory PmPlatformDiscovery::DiscoverInstalledPackages( const std::vector<PmProductDiscoveryRules> &catalogRules ) {
    
    assert(pkgUtilManager_);
    if (!pkgUtilManager_) {
        throw std::runtime_error("Invalid pkgUtilManager instance");
    }
    
    PackageInventory packagesDiscovered;
    const auto& packages = pkgUtilManager_->listPackages();
    for (const auto& rule : catalogRules) {
        for (const auto& pkgUtilRule : rule.pkgutil_discovery) {
            const auto& pkgId = pkgUtilRule.pkgId;
            const auto it = std::find(packages.begin(), packages.end(), pkgId);
            if (it != packages.end()) {
                const auto& pkgInfo = pkgUtilManager_->getPackageInfo(pkgId);
                packagesDiscovered.packages.push_back({ pkgId, pkgInfo.version });
            }
        }
    }
    
    return packagesDiscovered;
}
