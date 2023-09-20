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

} //unnamed namespace

CrashpadTuner* CrashpadTuner::instance_ = nullptr;

CrashpadTuner::CrashpadTuner():
    bRunning_(false),
    bUploadEnabled_(true),
    uploadUrl_(static_cast<std::string>(kCrashpadUrl)),
    nPruneDays_(kPruneDays),
    databasePruneSize_(kDatabaseDefaultPruneSizeKB)
{
}

CrashpadTuner* CrashpadTuner::getInstance()
{
    if (instance_ == nullptr)
    {
        instance_ = new CrashpadTuner();
        std::atexit(at_exit_handler);
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
}
