/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once
#include <string>
#include <functional>
#include <map>
#include <mutex>
#include <CoreServices/CoreServices.h>

class FileWatcher
{
public:
    explicit FileWatcher( const std::string& watcherName );
    virtual ~FileWatcher();
    FileWatcher(const FileWatcher&) = delete;

    bool add(const std::string& filePath, std::function<void()> Receiver);
    bool remove(const std::string& filePath);

private:
    std::map<std::string, FSEventStreamRef> streams_;
    dispatch_queue_t queue_;
    std::mutex streamLock_;
};
