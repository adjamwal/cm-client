#include "PmPlatformDiscovery.hpp"
#include <set>
#include <sys/utsname.h>
#include <cassert>

namespace
{
    auto determineArch = [](){
        std::string sPath;
        constexpr std::string_view x64{"x64"};
        constexpr std::string_view aarch64{"aarch64"};
        constexpr std::string_view arm64{"arm64"};
        struct utsname sysinfo{};
        uname(&sysinfo);
        sPath =  std::string(sysinfo.machine) == arm64
                                            ? aarch64
                                            : x64;
        
        return sPath;
    };

    static std::string sArchForDiscovery = determineArch();
}


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

    packagesDiscovered.architecture = sArchForDiscovery;
    packagesDiscovered.platform = "mac";

    lastDetectedPackages_ = packagesDiscovered;
    
    return packagesDiscovered;
}

PackageInventory PmPlatformDiscovery::CachedInventory() const {
    return lastDetectedPackages_;
}
