/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include <Foundation/Foundation.h>
#include <regex>
#include "FileUtilities.hpp"
#include "PmLogger.hpp"

namespace PackageManager
{
const std::unordered_map<std::string, unsigned int> NSSearchPathDirectoryMap::knownFolderMap = {
    { "ApplicationDirectory", NSApplicationDirectory },
    { "DemoApplicationDirectory", NSDemoApplicationDirectory },
    { "DeveloperApplicationDirectory", NSDeveloperApplicationDirectory },
    { "AdminApplicationDirectory", NSAdminApplicationDirectory },
    { "LibraryDirectory", NSLibraryDirectory },
    { "DeveloperDirectory", NSDeveloperDirectory },
    { "UserDirectory", NSUserDirectory },
    { "DocumentationDirectory", NSDocumentationDirectory },
    { "DocumentDirectory", NSDocumentDirectory },
    { "CoreServiceDirectory", NSCoreServiceDirectory },
    { "AutosavedInformationDirectory", NSAutosavedInformationDirectory },
    { "DesktopDirectory", NSDesktopDirectory },
    { "CachesDirectory", NSCachesDirectory },
    { "ApplicationSupportDirectory", NSApplicationSupportDirectory },
    { "DownloadsDirectory", NSDownloadsDirectory },
    { "InputMethodsDirectory", NSInputMethodsDirectory },
    { "MoviesDirectory", NSMoviesDirectory },
    { "MusicDirectory", NSMusicDirectory },
    { "PicturesDirectory", NSPicturesDirectory },
    { "PrinterDescriptionDirectory", NSPrinterDescriptionDirectory },
    { "SharedPublicDirectory", NSSharedPublicDirectory },
    { "PreferencePanesDirectory", NSPreferencePanesDirectory },
    { "ApplicationScriptsDirectory", NSApplicationScriptsDirectory },
    { "ItemReplacementDirectory", NSItemReplacementDirectory },
    { "AllApplicationsDirectory", NSAllApplicationsDirectory },
    { "AllLibrariesDirectory", NSAllLibrariesDirectory },
    { "TrashDirectory", NSTrashDirectory },
};

std::string ResolveKnownFolderIdForDomain(const std::string& knownFolderId, NSSearchPathDomainMask domain)
{
    std::string knownFolder;
    
    const auto it = NSSearchPathDirectoryMap::knownFolderMap.find(knownFolderId);
    if (it != NSSearchPathDirectoryMap::knownFolderMap.end()) {
        NSSearchPathDirectory directory = (NSSearchPathDirectory)(*it).second;
        NSArray* paths = NSSearchPathForDirectoriesInDomains(directory, domain, YES);
        if (paths.count > 0) {
            NSString* path = [paths objectAtIndex:0];
            knownFolder = [path UTF8String];
        } else {
            PM_LOG_WARNING("No search paths returned for %s", knownFolderId.c_str());
        }
    } else {
        PM_LOG_WARNING("No knownfolderID found to match %s", knownFolderId.c_str());
    }
    
    return knownFolder;
}


std::string FileUtilities::ResolveKnownFolderIdForDefaultUser(const std::string& knownFolderId)
{
    return ResolveKnownFolderIdForDomain(knownFolderId, NSSystemDomainMask);
}
    
}
