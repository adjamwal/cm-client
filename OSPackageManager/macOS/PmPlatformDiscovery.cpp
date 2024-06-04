#include "PmPlatformDiscovery.hpp"
#include "PmLogger.hpp"
#include <set>
#include <sys/utsname.h>
#include <cassert>

namespace
{
    auto determineArch = [](){
        std::string sPath;
        constexpr std::string_view x64{"amd64"};
        constexpr std::string_view arm64{"arm64"};
        struct utsname sysinfo{};
        uname(&sysinfo);
        sPath =  std::string(sysinfo.machine) == arm64
                                            ? arm64
                                            : x64;
        
        return sPath;
    };

    static std::string sArchForDiscovery = determineArch();
}

void PmPlatformDiscovery::ResolveAndDiscover(
    const std::filesystem::path& unresolvedPath,
    const std::filesystem::path& resolvedPath,
    std::string& out_knownFolderId,
    std::string& out_knownFolderIdConversion,
    std::vector<std::filesystem::path>& out_discoveredFiles )
{
    out_knownFolderId = "";
    out_knownFolderIdConversion = "";
    out_discoveredFiles.clear();

    if( unresolvedPath != resolvedPath )
    {
        //Resolved path is deferent which means we must calculate the knownfolderid
        std::string tempResolvedPath = resolvedPath.generic_u8string();
        std::string tempUnresolvedPath = unresolvedPath.generic_u8string();
        size_t first = tempUnresolvedPath.find( "<FOLDERID_" );
        size_t last = tempUnresolvedPath.find_first_of( ">" );
        if ( std::string::npos != first && std::string::npos != last ) {
            out_knownFolderId = tempUnresolvedPath.substr( first, last + 1 );
            std::string remainingPath = tempUnresolvedPath.substr( last + 1, tempUnresolvedPath.length() );

            first = tempResolvedPath.find( remainingPath );

            out_knownFolderIdConversion = tempResolvedPath.substr( 0, first );
        }
    }

    fileUtils_->FileSearchWithWildCard( resolvedPath, out_discoveredFiles );
    //WindowsUtilities::FileSearchWithWildCard( resolvedPath, out_discoveredFiles );
    //m_utf8PathVerifier.PruneInvalidPathsFromList( out_discoveredFiles );
    for( auto it = out_discoveredFiles.begin(); it != out_discoveredFiles.end(); ) {
        if( !fileUtils_->PathIsValid( *it ) ) {
            it = out_discoveredFiles.erase( it );
        }
        else {
            it++;
        }
    }
}

void PmPlatformDiscovery::DiscoverPackageConfigurables(
    const std::vector<PmProductDiscoveryConfigurable>& configurables,
    std::vector<PackageConfigInfo>& packageConfigs )
{
    for( auto& configurable : configurables ) {
        std::string knownFolderId = "";
        std::string knownFolderIdConversion = "";
        std::vector<std::filesystem::path> discoveredFiles;
        bool usingDeployPath = false;

        if( !configurable.deployPath.empty() ) {
            ResolveAndDiscover(
                configurable.unresolvedDeployPath,
                configurable.deployPath,
                knownFolderId,
                knownFolderIdConversion,
                discoveredFiles
            );

            usingDeployPath = discoveredFiles.size() > 0;
        }

        if( !usingDeployPath ) {
            ResolveAndDiscover(
                configurable.unresolvedCfgPath,
                configurable.cfgPath,
                knownFolderId,
                knownFolderIdConversion,
                discoveredFiles
            );
        }

        if( discoveredFiles.size() > configurable.max_instances ) {
            if( configurable.max_instances == 0 ) {
                discoveredFiles = std::vector<std::filesystem::path>( discoveredFiles.begin(), discoveredFiles.begin() + 1 );
            }
            else {
                discoveredFiles = std::vector<std::filesystem::path>( discoveredFiles.begin(), discoveredFiles.begin() + configurable.max_instances );
            }
        }

        for( auto& discoveredFile : discoveredFiles ) {
            PackageConfigInfo configInfo = {};
            bool uniqueConfigurable = true;

            std::string tempPath = discoveredFile.generic_u8string();
            if( knownFolderId != "" ) {
                //We need to convert the path to include knownfolderid
                tempPath = tempPath.substr( knownFolderIdConversion.length(), tempPath.length() );
                tempPath = knownFolderId + tempPath;
            }

            configInfo.isDiscoveredAtDeployPath = usingDeployPath;

            if( usingDeployPath ) {
                configInfo.deployPath = discoveredFile;
                configInfo.unresolvedDeployPath = std::filesystem::u8path( tempPath );
                configInfo.cfgPath = "";
                // Provide the original config path as this is needs to be sent in the checkin
                configInfo.unresolvedCfgPath = configurable.unresolvedCfgPath;
            }
            else {
                configInfo.deployPath = "";
                configInfo.unresolvedDeployPath = "";
                configInfo.cfgPath = discoveredFile;
                configInfo.unresolvedCfgPath = std::filesystem::u8path( tempPath );
            }

            for( auto& it : packageConfigs ) {
                if( usingDeployPath && ( configInfo.deployPath == it.deployPath ) ) {
                    uniqueConfigurable = false;
                    break;
                }
                else if( !configInfo.cfgPath.empty() && ( configInfo.cfgPath == it.cfgPath ) ) {
                    uniqueConfigurable = false;
                    break;
                } 
                else if( usingDeployPath && ( configInfo.deployPath == it.cfgPath ) ) {
                    PM_LOG_ERROR( "Deploy path %s matches a previous config path. Bad cloud configuration", configInfo.deployPath.c_str() );
                    uniqueConfigurable = false;
                    break;
                }
                else if( !configInfo.cfgPath.empty() && ( configInfo.cfgPath == it.deployPath) ) {
                    PM_LOG_ERROR( "Deploy path %s matches a previous config path. Bad cloud configuration", configInfo.cfgPath.c_str() );
                    uniqueConfigurable = false;
                    break;
                }
            }
            
            if( uniqueConfigurable ) {
                packageConfigs.push_back( configInfo );
            }
        }
    }
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

            //Backend requires strictly only single instance in checking request
            if ( uniquePks.end() != uniquePks.find(rule.product) )
                continue;
                
            uniquePks.insert(rule.product);

            std::vector<PackageConfigInfo> configs;
            DiscoverPackageConfigurables ( rule.configurables, configs );
            
            PM_LOG_INFO( "Discovered package %s, version %s, found %d configurables", pkgId.c_str(), pkgInfo.version.c_str(), configs.size() );
            packagesDiscovered.packages.push_back({ rule.product, pkgInfo.version, configs });
        }
    }

    packagesDiscovered.architecture = sArchForDiscovery;
    packagesDiscovered.platform = "darwin";

    lastDetectedPackages_ = packagesDiscovered;
    
    return packagesDiscovered;
}

PackageInventory PmPlatformDiscovery::CachedInventory() const {
    return lastDetectedPackages_;
}
