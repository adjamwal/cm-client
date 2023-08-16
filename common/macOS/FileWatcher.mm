/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "FileWatcher.hpp"
#include <iostream>
#import <Foundation/Foundation.h>

FileWatcher::FileWatcher(const std::string& watcherName)
{
    queue_ = dispatch_queue_create(watcherName.c_str(), DISPATCH_QUEUE_SERIAL);
}

FileWatcher::~FileWatcher()
{
    while (!streams_.empty()){
        remove(streams_.begin()->first);
    }
}

namespace {
struct ContextData
{
    std::function<void()> receiver_;

    static void dispose(const void *ctx) {
      delete static_cast<const ContextData *>(ctx);
    }
};

void CallbackFunction( ConstFSEventStreamRef theStream, void* clientCallBackInfo, size_t numEvents, void* eventPaths,
                       const FSEventStreamEventFlags flags[], const FSEventStreamEventId ids[])
{
    (void) theStream;
    (void) ids;

    constexpr const FSEventStreamEventFlags StreamInvalidatingFlags =
    kFSEventStreamEventFlagUserDropped | kFSEventStreamEventFlagKernelDropped |
    kFSEventStreamEventFlagMustScanSubDirs;

    constexpr const FSEventStreamEventFlags ModifyingFileEvents =
    kFSEventStreamEventFlagItemCreated | kFSEventStreamEventFlagItemRenamed |
    kFSEventStreamEventFlagItemModified;

    for (size_t i = 0; i < numEvents; ++i) {
        const FSEventStreamEventFlags Flags = flags[i];
        if (Flags & ModifyingFileEvents) {
            if (auto *ctx = static_cast<ContextData*>(clientCallBackInfo)) {
                const std::string sPath{((const char **)eventPaths)[i]};
                //TODO: return once possible
                //PM_LOG_DEBUG("Invoking file hanlder receiver for path: %s", sPath.c_str());
                ctx->receiver_(); //that's where main work is done
            }
        } else if (Flags & StreamInvalidatingFlags) {
            continue;
        } else if (!(Flags & kFSEventStreamEventFlagItemIsFile)) {
            //happens on subdir, not applicable for our case
            continue;
        } else if (Flags & kFSEventStreamEventFlagItemRemoved) {
            continue;
        } else if (Flags & kFSEventStreamEventFlagItemInodeMetaMod) {
            //usually happens on "touch" operations
            continue;
        } else {
            continue;
        }
    }
}

}

bool FileWatcher::add(const std::string& filePath, std::function<void()> Receiver)
{
    std::lock_guard<std::mutex> guard(streamLock_);

    if (filePath.empty() || streams_.end() != streams_.find(filePath) || !Receiver)
        return false;

    NSString *thePath = [NSString stringWithUTF8String:filePath.c_str()];
    NSArray *fileArray = @[thePath];
    FSEventStreamContext ctxEvt{0, new ContextData{std::move(Receiver)}, nullptr, ContextData::dispose, nullptr};
    auto streamFS = FSEventStreamCreate(kCFAllocatorDefault, &CallbackFunction, &ctxEvt, (__bridge CFArrayRef)fileArray, kFSEventStreamEventIdSinceNow, 0, kFSEventStreamCreateFlagNoDefer | kFSEventStreamCreateFlagFileEvents );

    if (!streamFS)
        return false;

    streams_.insert({filePath, streamFS});
    auto initStreamQueue = ^{
        FSEventStreamSetDispatchQueue(streamFS, queue_);
        FSEventStreamStart(streamFS);
    };
    dispatch_sync(queue_, initStreamQueue);

    //TODO: return once possible
    //PM_LOG_INFO("Adding file %s to watched list", filePath.c_str());

    return true;
}

bool FileWatcher::remove(const std::string &filePath)
{
    std::lock_guard<std::mutex> guard(streamLock_);

    if ( filePath.empty() || streams_.end() == streams_.find(filePath) )
        return false;

    auto streamIt = streams_.find(filePath);
    if (streams_.end() == streamIt || !streamIt->second )
        return false;

    auto removeEventFromQueue = ^{
        FSEventStreamStop(streamIt->second);
        FSEventStreamInvalidate(streamIt->second);
        FSEventStreamRelease(streamIt->second);
    };
    dispatch_sync(queue_, removeEventFromQueue);

    streams_.erase(streamIt);

    //TODO: return once possible
    //PM_LOG_INFO("Removing file %s from watched list", filePath.c_str());

    return  true;
}
