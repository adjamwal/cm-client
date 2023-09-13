#include "PmPlatformDiscovery.hpp"
#include <set>

PackageInventory PmPlatformDiscovery::DiscoverInstalledPackages( const std::vector<PmProductDiscoveryRules> &catalogRules ) {
    
    assert(pkgUtilManager_);
    if (!pkgUtilManager_) {
        throw std::runtime_error("Invalid pkgUtilManager instance");
    }
    
    std::set<std::string> uniquePks;
    PackageInventory packagesDiscovered;
    const auto& packages = pkgUtilManager_->listPackages();
    for (const auto& rule : catalogRules) {
        for (const auto& pkgUtilRule : rule.pkgutil_discovery) {
            const auto& pkgId = pkgUtilRule.pkgId;
            if ( std::end( packages ) == std::find(packages.begin(), packages.end(), pkgId))
                continue;
            
            const auto& pkgInfo = pkgUtilManager_->getPackageInfo(pkgId);
            //Backend requires strictly only single instace in checking request
            if ( uniquePks.end() != uniquePks.find(pkgId) )
                continue;

            uniquePks.insert(pkgId);
            packagesDiscovered.packages.push_back({ pkgId, pkgInfo.version });
        }
    }
    //TODO: Add proper values once ready in https://jira-eng-rtp3.cisco.com/jira/browse/CM4E-291
    packagesDiscovered.architecture = "x64";
    packagesDiscovered.platform = "mac";

    lastDetectedPackages_ = packagesDiscovered;
    
    return packagesDiscovered;
}

PackageInventory PmPlatformDiscovery::CachedInventory() const {
    return lastDetectedPackages_;
}
