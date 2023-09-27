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
    return ResolveKnownFolderIdForDomain(knownFolderId, NSLocalDomainMask);
}


/**
 * @brief Walks through nodes in XML of specific (.plist) format and modifies integer values behind "integer", where "selected" 
 * and feature names from map are present as strings
 * The XML structure is described in official manual:
 *  https://www.cisco.com/c/en/us/support/docs/security/anyconnect-secure-mobility-client/215876-customize-anyconnect-module-installation.html
 *
 * @return true if anything has been changed, false otherwise
 */
bool modifyXmlValues(const std::string& sFilePath, const std::map<std::string, int>&  installOptions)
{
    NSString * spath = [NSString stringWithUTF8String:sFilePath.c_str()];
    if(!spath || [spath length] == 0)
        return false;

    NSXMLDocument* doc = [[NSXMLDocument alloc] initWithContentsOfURL: [NSURL fileURLWithPath:spath] options:0 error:NULL];
    if(!doc)
        return false;

    NSXMLElement* root  = [doc rootElement];
    if(!root)
        return false;
    
    //Get straight down to array of dictionaries
    NSArray* dateArray = [root nodesForXPath:@"//plist//array//dict" error:nil];
    if(!dateArray)
        return false;

    bool bDidChange = false;
    for(NSXMLElement* xmlElement in dateArray) {
        //Integer fields contain actual feature value, while string contains its name,
        //same name as map's key
        NSArray* intArray = [xmlElement nodesForXPath:@"integer" error:nil];
        NSArray* strArray = [xmlElement nodesForXPath:@"string" error:nil];
        if ( !(intArray && [intArray count] > 0 && strArray && [strArray count] >= 2))
            continue;

        NSXMLNode *intNode = [intArray objectAtIndex:0];
        if (!intNode)
            continue;

        auto name = [intNode name];
        if ( ![name isEqual:@"integer"] )
            continue;

        NSMutableArray *nameArray = [[NSMutableArray alloc] initWithCapacity:[strArray count]];
        for(NSUInteger i=0; i<[strArray count]; i++)
            [nameArray addObject:[[strArray objectAtIndex:i] stringValue]];

        //Map keys are expected to be the same as values for XML nodes
        for (auto &&[feat_name, feat_val]: installOptions) {
            if ( !([nameArray containsObject:[NSString stringWithUTF8String:feat_name.c_str()]]
                && [nameArray containsObject:@"selected"]))
                continue;

            //If any of feature is identified, modify its value to the same as in input map
            auto sval = std::to_string(feat_val);
            [intNode setStringValue:[NSString stringWithUTF8String:sval.c_str()]];
            bDidChange = true;
        }
    }

    //And write modified XML to same file back, so it can be fed into installer further
    NSData *xmlData = [doc XMLDataWithOptions:NSXMLNodePrettyPrint];
    [xmlData writeToFile:spath atomically:YES];

    return bDidChange;
}
    
}
