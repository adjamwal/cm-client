/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once

#include <memory>
#include <map>
#include <vector>
#include <list>
#include <mutex>
#include <string>

#include "ProxyDiscovery/IProxyDiscoveryEngine.h"

#include "base/files/file_path.h"
#include "util/net/http_proxy.h"

namespace crashpad {
class CrashpadClient;
class CrashReportDatabase;
}

class CrashpadTuner: public proxy::IProxyObserver
{
public:
    CrashpadTuner();
    virtual ~CrashpadTuner() = default;
    void init(const std::string& strBinDirPath);
    
    static CrashpadTuner* getInstance();
    static void deleteInstance();
    
    void setUploadEnabled(bool bVal);
    void setUploadUrl(const std::string& strUrl);
    void setAgentGuid(const std::string& strGuid);
    // Prune period in days.
    void setPruneAge(uint32_t nDays);
    // Size of the crash database in KB.
    void setPruneDatabaseSize(size_t nSize);
    bool setProxy(const crashpad::HTTPProxy& proxy);

    std::string getUploadUrl() const;
    crashpad::HTTPProxy getProxy() const;
    
    void startProxyDiscoveryAsync();
    
    //IProxyObserver methods
    void updateProxyList(const std::list<proxy::ProxyRecord>& proxies, const std::string& guid) override;

private:
    std::unique_ptr<crashpad::CrashReportDatabase> pDatabase_;
    std::unique_ptr<crashpad::CrashpadClient> pClient_;
    static CrashpadTuner* instance_;
    static std::mutex g_mutex;
    std::map<std::string, std::string> annotations_;
    std::vector<std::string> arguments_;
    std::vector<base::FilePath> attachments_;
    bool bRunning_;
    
    //settings
    bool bUploadEnabled_;
    std::string uploadUrl_;
    std::string clientId_;
    int nPruneDays_;
    size_t databasePruneSize_;
    crashpad::HTTPProxy proxy_;
    
    std::shared_ptr<proxy::IProxyDiscoveryEngine> pProxyEngine_;
    mutable std::recursive_mutex mutex_;
    
    void fillAnnotations();
    void fillArguments();
    void fillAttachments();
    bool tuneReportDirectory(const std::string& strDir);
    void setupSettings();
    bool setProxySettings(const crashpad::HTTPProxy& proxy);
};
