/**
 * @file
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "crashpad/CrashpadTuner.h"

#include "Config.hpp"
#include "Logger/CMLogger.hpp"
#include "util/PathUtil.hpp"

#include "client/crashpad_client.h"
#include "client/crash_report_database.h"
#include "client/settings.h"

#include <cstdlib>
#include <filesystem>
#include <limits>

namespace
{
void at_exit_handler()
{
    CrashpadTuner::deleteInstance();
}

constexpr std::string_view kCrashpadPath{"/opt/cisco/secureclient/cloudmanagement/ch"};
constexpr std::string_view kNoRateLimit{"--no-rate-limit"};
constexpr std::string_view kNoUploadGZip{"--no-upload-gzip"};
constexpr std::string_view kMinidumpFormat{"minidump"};
constexpr std::string_view kCrashPadExecutable{"crashpad_handler"};
constexpr std::string_view kSecureClientCloudManagementMacClient{"sccm_mac"};
constexpr uint32_t kPruneDays{14};
constexpr size_t kDatabaseDefaultPruneSizeKB{50000};
constexpr std::string_view kNotUsedUrl{"https://default.not.used.url/"};
constexpr std::string_view kCrashpadUrl{"https://crash.amp.cisco.com/crash"};

crashpad::HTTPProxy makeEmptyProxy()
{
    return crashpad::HTTPProxy("",
    0,
    crashpad::HTTPProxy::kProxyNone,
    crashpad::HTTPProxy::kProxyAuthNone,
    "", "");
}

bool operator==(const crashpad::HTTPProxy& a, const crashpad::HTTPProxy& b) {
    return a.GetHost() == b.GetHost() &&
        a.GetPort() == b.GetPort() &&
        a.GetType() == b.GetType() &&
        a.GetAuth() == b.GetAuth() &&
        a.GetUsername() == b.GetUsername() &&
        a.GetPassword() == b.GetPassword();
}

bool isProxyValid(const crashpad::HTTPProxy& proxy)
{
    if (proxy.GetType() == crashpad::HTTPProxy::kProxyNone)
    {
        return true;
    }
    if (proxy.GetHost().empty() || proxy.GetPort() == 0)
    {
        return false;
    }
    if (proxy.GetAuth() != crashpad::HTTPProxy::kProxyAuthNone)
        return !proxy.GetUsername().empty() && !proxy.GetPassword().empty();
    
    return true;
}

std::list<crashpad::HTTPProxy> convertProxies(const std::list<proxy::ProxyRecord>& inputProxies)
{
    std::list<crashpad::HTTPProxy> outputList;
    for (const auto& inputProxy: inputProxies)
    {
        crashpad::HTTPProxy::Type proxyType = crashpad::HTTPProxy::kProxyNone;
        switch (inputProxy.proxyType)
        {
        case proxy::ProxyTypes::SOCKS:
            //TODO (CM4E-300): not sure how to determine what Sockets protocol to use here:
            //SOCKS4, SOCKS4a, SOCKS5?
            proxyType = crashpad::HTTPProxy::kProxySOCKS5;
            break;
        case proxy::ProxyTypes::HTTPS:
            proxyType = crashpad::HTTPProxy::kProxyHTTPS;
            break;
        case proxy::ProxyTypes::HTTP:
            proxyType = crashpad::HTTPProxy::kProxyHTTP;
            break;
        case proxy::ProxyTypes::None:
            proxyType = crashpad::HTTPProxy::kProxyHTTP;
            break;
        case proxy::ProxyTypes::FTP:
            CM_LOG_ERROR("Seems FTP proxy is not supported by crashpad.");
            continue;
        default:
            CM_LOG_ERROR("Unsupported proxy type: %d", inputProxy.proxyType);
            continue;
        }
        uint16_t port = 0;
        if (inputProxy.port > static_cast<uint32_t>(std::numeric_limits<uint16_t>::max()))
        {
            CM_LOG_ERROR("Too big proxy port value: %d", inputProxy.port);
            continue;
        }
        port = static_cast<uint16_t>(inputProxy.port);
        // TODO ProxyDiscovery(CM4E-299): Upate ProxyDiscovery-Mac library to filter out
        // proxies that require authentication.
        crashpad::HTTPProxy::Auth auth = crashpad::HTTPProxy::Auth::kProxyAuthNone;
        outputList.emplace_back(inputProxy.url, port, proxyType,
                             auth, "", "");
    }
    return outputList;
}

} //unnamed namespace

CrashpadTuner* CrashpadTuner::instance_ = nullptr;
std::mutex CrashpadTuner::g_mutex;

CrashpadTuner::CrashpadTuner():
    bRunning_(false),
    bUploadEnabled_(true),
    uploadUrl_(static_cast<std::string>(kCrashpadUrl)),
    nPruneDays_(kPruneDays),
    databasePruneSize_(kDatabaseDefaultPruneSizeKB),
    proxy_(makeEmptyProxy()),
    pProxyEngine_(proxy::createProxyEngine())
{
    pProxyEngine_->addObserver(this);
}

CrashpadTuner* CrashpadTuner::getInstance()
{
    if (instance_ == nullptr)
    {
        std::lock_guard<std::mutex> guard(g_mutex);
        if (instance_ == nullptr)
        {
            instance_ = new CrashpadTuner();
            std::atexit(at_exit_handler);
        }
    }
    return instance_;
}

void CrashpadTuner::deleteInstance()
{
    delete instance_;
}

void CrashpadTuner::init(const std::string& strBinDirPath)
{
    std::filesystem::path pathCrashpadExecutable = std::filesystem::path{strBinDirPath} / std::filesystem::path{static_cast<std::string>(kCrashPadExecutable)};
    if (!std::filesystem::exists(pathCrashpadExecutable))
    {
        CM_LOG_ERROR("Can't find crashpad_handler by this path: %s", pathCrashpadExecutable.native().c_str());
        return;
    }
    
    //init crashpad path
    std::string strCrashpadPath = static_cast<std::string>(kCrashpadPath);
    if (!tuneReportDirectory(strCrashpadPath))
        return;

    fillAnnotations();
    fillArguments();
    fillAttachments();
    
    pDatabase_ = crashpad::CrashReportDatabase::Initialize(
        base::FilePath{strCrashpadPath});
    if (!pDatabase_)
    {
        CM_LOG_ERROR("Unable to create crashpad report database in the following directory: %s", strCrashpadPath.c_str());
        return;
    }
    setupSettings();
    
    pClient_ = std::make_unique<crashpad::CrashpadClient>();
    
    bool status = pClient_->StartHandler(base::FilePath{pathCrashpadExecutable.native()},
                base::FilePath{strCrashpadPath},
                base::FilePath{strCrashpadPath},
                static_cast<std::string>(kNotUsedUrl),
                annotations_,
                arguments_,
                true,
                true,
                attachments_);
    if (!status) {
        CM_LOG_ERROR("Failed to start crashpad handler!");
    } else {
        bRunning_ = true;
    }
}

void CrashpadTuner::fillAnnotations()
{
    annotations_.clear();
    
    annotations_["format"] = static_cast<std::string>(kMinidumpFormat);
    std::string strAppName = util::getApplicationName();
    annotations_["database"] = "database_" + strAppName;
    annotations_["product"] = strAppName;
    annotations_["module"] = static_cast<std::string>(kSecureClientCloudManagementMacClient);
    annotations_["version"] = util::getApplicationVersion();
}

void CrashpadTuner::fillArguments()
{
    arguments_.clear();
    arguments_.push_back(static_cast<std::string>(kNoRateLimit));
    arguments_.push_back(static_cast<std::string>(kNoUploadGZip));
}

void CrashpadTuner::fillAttachments()
{
    attachments_.clear();
    //TODO CM4E-272: add implementation if needed to store log files.
}

bool CrashpadTuner::tuneReportDirectory(const std::string& strCrashpadPath)
{
    auto crashpadPath = std::filesystem::path{strCrashpadPath};
    if (!std::filesystem::exists(crashpadPath))
    {
        std::error_code errCode;
        bool bDirExists = std::filesystem::create_directories(crashpadPath, errCode);
        if (!bDirExists)
        {
            CM_LOG_ERROR("Failed to create directory %s, for the log file, error code: %d", strCrashpadPath.c_str(), errCode.value());
            return false;
        }
    }
    std::error_code errCode;
    std::filesystem::permissions(crashpadPath,
        std::filesystem::perms::owner_write,
        std::filesystem::perm_options::add, errCode);
    if (errCode)
    {
        CM_LOG_ERROR("Failed to set write permissions for the sirectory: %s, for the log file, error code: %d", strCrashpadPath.c_str(), errCode.value());
        return false;
    }
    return true;
}

void CrashpadTuner::setUploadEnabled(bool bVal)
{
    bUploadEnabled_ = bVal;
    if (!bRunning_)
        return;
    
    if (!pDatabase_)
        return;
    
    crashpad::Settings* pSettings = pDatabase_->GetSettings();
    if (!pSettings)
    {
        CM_LOG_ERROR("Failed to get settings from the database");
        return;
    }
    
    bool bCurEnable;
    if (pSettings->GetUploadsEnabled(&bCurEnable) && bCurEnable == bVal)
        return;
    if (!pSettings->SetUploadsEnabled(bVal))
        CM_LOG_ERROR("Failed to set uploads enabled.");
}

void CrashpadTuner::setUploadUrl(const std::string& strUrl)
{
    uploadUrl_ = strUrl;
    if (!bRunning_)
        return;
    
    if (!pDatabase_)
        return;
    
    crashpad::Settings* pSettings = pDatabase_->GetSettings();
    if (!pSettings)
    {
        CM_LOG_ERROR("Failed to get settings from the database");
        return;
    }
    
    std::string curUrl;
    if (pSettings->GetUploadUrl(curUrl) && curUrl.compare(strUrl) == 0)
        return;
    if (!pSettings->SetUploadUrl(strUrl))
        CM_LOG_ERROR("Failed to set upload url.");
    
    CM_LOG_INFO("Applied crashpad upload url = %s", strUrl.c_str());
}

void CrashpadTuner::setAgentGuid(const std::string& strGuid)
{
    if (strGuid.empty())
        return;

    clientId_ = strGuid;
    if (!bRunning_)
        return;

    if (!pDatabase_)
        return;

    crashpad::Settings* pSettings = pDatabase_->GetSettings();
    if (!pSettings)
    {
        CM_LOG_ERROR("Failed to get settings from the database");
        return;
    }

    crashpad::UUID curId;
    crashpad::UUID inputId;
    if (!inputId.InitializeFromString(strGuid.c_str()))
        return;

    if (pSettings->GetClientID(&curId) && curId == inputId)
        return;

    if (!pSettings->SetClientID(inputId))
        CM_LOG_ERROR("Failed to set client id.");
}

void CrashpadTuner::setPruneAge(uint32_t nDays)
{
    if (nDays > std::numeric_limits<int>::max())
    {
        CM_LOG_ERROR("Input prune period value is bigger than INT_MAX: %d", nDays);
        return;
    }
    nPruneDays_ = static_cast<int>(nDays);
    if (!bRunning_)
        return;

    if (!pDatabase_)
        return;

    crashpad::Settings* pSettings = pDatabase_->GetSettings();
    if (!pSettings)
    {
        CM_LOG_ERROR("Failed to get settings from the database");
        return;
    }

    int curAge;
    if (pSettings->GetPruneAge(&curAge) && curAge == nPruneDays_)
        return;

    if (!pSettings->SetPruneAge(nPruneDays_)) {
        CM_LOG_ERROR("Failed to set prune age.");
    }
    
    CM_LOG_INFO("Applied crashpad pruneAge = %d days", nDays);
}

void CrashpadTuner::setPruneDatabaseSize(size_t nSize)
{
    databasePruneSize_ = nSize;
    if (!bRunning_)
        return;
    
    if (!pDatabase_)
        return;
    
    crashpad::Settings* pSettings = pDatabase_->GetSettings();
    if (!pSettings)
    {
        CM_LOG_ERROR("Failed to get settings from the database");
        return;
    }
    
    size_t curSize;
    if (pSettings->GetPruneDatabaseSize(&curSize) && curSize == nSize)
        return;
    if (!pSettings->SetPruneDatabaseSize(nSize))
        CM_LOG_ERROR("Failed to set prune database size.");
    
    CM_LOG_INFO("Applied crashpad database prune size = %d kb", nSize);
}

void CrashpadTuner::setupSettings()
{
    if (!pDatabase_)
        return;

    crashpad::Settings* pSettings = pDatabase_->GetSettings();
    if (!pSettings)
    {
        CM_LOG_ERROR("Failed to get settings from the database");
        return;
    }
    pSettings->SetUploadsEnabled(bUploadEnabled_);
    pSettings->SetUploadUrl(uploadUrl_);

    if (!clientId_.empty())
    {
        crashpad::UUID inputId;
        if (!inputId.InitializeFromString(clientId_.c_str()))
        {
            CM_LOG_ERROR("Failed to convert string to guid, string: %s", clientId_.c_str());
        }
        else
        {
            pSettings->SetClientID(inputId);
        }
    }

    pSettings->SetPruneAge(nPruneDays_);
    pSettings->SetPruneDatabaseSize(databasePruneSize_);
    setProxySettings(proxy_);
}

bool CrashpadTuner::setProxy(const crashpad::HTTPProxy& proxy)
{
    std::lock_guard<std::recursive_mutex> lk(mutex_);
    if (!isProxyValid(proxy))
    {
        CM_LOG_NOTICE("Input proxy object is not valid.");
        return false;
    }

    if (!pDatabase_)
    {
        proxy_ = proxy;
        return true;
    }
    
    crashpad::HTTPProxy currentProxy = getProxy();
    if (currentProxy == proxy)
    {
        CM_LOG_NOTICE("Attempt to set the same proxy as already was set.");
        return true;
    }
    return setProxySettings(proxy);
}

bool CrashpadTuner::setProxySettings(const crashpad::HTTPProxy& proxy)
{
    if (!pDatabase_)
        return false;

    crashpad::Settings* pSettings = pDatabase_->GetSettings();
    if (!pSettings)
    {
        CM_LOG_ERROR("Failed to get settings from the database");
        return false;
    }
    
    crashpad::HTTPProxy::Type proxyType = proxy.GetType();
    if (!pSettings->SetUploadProxyType(proxyType))
    {
        CM_LOG_ERROR("Failed to set proxy type to settings.");
        return false;
    }
    
    if (proxyType == crashpad::HTTPProxy::kProxyNone)
    {
        //return earlier
        return true;
    }
    
    if (!pSettings->SetUploadProxyHost(proxy.GetHost()))
    {
        CM_LOG_ERROR("Failed to set proxy host to settings: %s", proxy.GetHost().c_str());
        return false;
    }
    
    if (!pSettings->SetUploadProxyPort(proxy.GetPort()))
    {
        CM_LOG_ERROR("Failed to set proxy port to settings: %d", proxy.GetPort());
        return false;
    }
    
    uint32_t auth = proxy.GetAuth();
    if (!pSettings->SetUploadProxyAuth(auth))
    {
        CM_LOG_ERROR("Failed to set proxy auth to settings: %d", auth);
        return false;
    }
    if (auth == crashpad::HTTPProxy::kProxyAuthNone)
    {
        //return earlier
        return true;
    }
    
    if (!pSettings->SetUploadProxyUsername(proxy.GetUsername()))
    {
        CM_LOG_ERROR("Failed to set proxy username to settings");
        return false;
    }
    
    if (!pSettings->SetUploadProxyPassword(proxy.GetPassword()))
    {
        CM_LOG_ERROR("Failed to set proxy password to settings");
        return false;
    }
    return true;
}

crashpad::HTTPProxy CrashpadTuner::getProxy() const
{
    std::lock_guard<std::recursive_mutex> lk(mutex_);
    if (!pDatabase_)
        return proxy_;
    
    crashpad::HTTPProxy proxy = makeEmptyProxy();
    
    crashpad::Settings* pSettings = pDatabase_->GetSettings();
    if (!pSettings)
    {
        CM_LOG_ERROR("Failed to get settings from the database");
        return proxy_;
    }
    std::string strHost;
    if (!pSettings->GetUploadProxyHost(strHost))
    {
        CM_LOG_ERROR("Failed to get proxy host from settings.");
        return proxy_;
    }
    proxy.SetHost(strHost);
    
    uint16_t port;
    if (!pSettings->GetUploadProxyPort(&port))
    {
        CM_LOG_ERROR("Failed to get proxy port from settings.");
        return proxy_;
    }
    proxy.SetPort(port);
    
    crashpad::HTTPProxy::Type proxyType = crashpad::HTTPProxy::kProxyNone;
    if (!pSettings->GetUploadProxyType(&proxyType))
    {
        CM_LOG_ERROR("Failed to get proxy type from settings.");
        return proxy_;
    }
    proxy.SetType(proxyType);
    
    uint32_t auth;
    if (!pSettings->GetUploadProxyAuth(&auth))
    {
        CM_LOG_ERROR("Failed to get proxy auth from settings.");
        return proxy_;
    }
    proxy.SetAuth(auth);
    
    std::string username;
    if (pSettings->GetUploadProxyUsername(username))
    {
        proxy.SetUsername(username);
    }
    
    std::string password;
    if (pSettings->GetUploadProxyPassword(password))
    {
        proxy.SetPassword(password);
    }
    
    return proxy;
}

std::string CrashpadTuner::getUploadUrl() const
{
    std::string url = uploadUrl_;
    if (!pDatabase_)
        return url;
    
    crashpad::Settings* pSettings = pDatabase_->GetSettings();
    if (!pSettings)
    {
        CM_LOG_ERROR("Failed to get settings from the database");
        return url;
    }
    if (!pSettings->GetUploadUrl(url))
    {
        CM_LOG_ERROR("Failed to get upload url from the database settings.");
    }
    return url;
}

void CrashpadTuner::updateProxyList(const std::list<proxy::ProxyRecord>& proxies, const std::string& guid)
{
    if (!guid.empty())
        return;
    
    std::list<crashpad::HTTPProxy> httpProxies = convertProxies(proxies);
    if (httpProxies.empty())
    {
        crashpad::HTTPProxy proxy = makeEmptyProxy();
        setProxy(proxy);
        return;
    }
    for (const auto& proxy: httpProxies)
    {
        if (setProxy(proxy))
            break;
    }
}

void CrashpadTuner::startProxyDiscoveryAsync()
{
    pProxyEngine_->waitPrevOpCompleted();
    pProxyEngine_->requestProxiesAsync(getUploadUrl(), "", "");
}
