#include "PackageManager/PmTypes.h"

class IPmPlatformDiscovery {
public:
    virtual ~IPmPlatformDiscovery() = default;
    virtual PackageInventory DiscoverInstalledPackages( const std::vector<PmProductDiscoveryRules> &catalogRules ) = 0;
};

