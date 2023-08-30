/**
 * @file
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "util/ScopedGuard.hpp"
#include "PmCertRetrieverImpl.hpp"
#include "PmLogger.hpp"

#include <CoreFoundation/CFArray.h>
#include <Security/Security.h>
#include <string>


namespace PackageManager
{
const std::string sSystemRootPath("/System/Library/Keychains/SystemRootCertificates.keychain");

bool PmCertRetrieverImpl::LoadSystemSslCertificates()
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

    SecKeychainRef keychain = nullptr;
    OSStatus statusChain = SecKeychainOpen(sSystemRootPath.c_str(), &keychain);
    auto guard = util::scoped_guard([=]() {
        if (keychain)
            CFRelease(keychain);
    });
    
    if ( errSecSuccess != statusChain )
    {
        PM_LOG_ERROR("Failed to open keychain with code: %d", statusChain);
        return false;
    }

    SecKeychainStatus keychainStatus = 0;
    statusChain = SecKeychainGetStatus(keychain, &keychainStatus);

#pragma clang diagnostic pop

    if (statusChain != errSecSuccess) {
        PM_LOG_ERROR("Failed to retrieve keychain status with code: %d", statusChain);
        return false;
    }

    if ( ! (keychainStatus & kSecReadPermStatus) ) {
        PM_LOG_ERROR("Keychain has no read permissions: %d", keychainStatus);
        return false;
    }
    
    /*
     * The SystemRootCertificates.keychain is a system keychain file that should be locked
     * and should definitely not have writable permissions.  This may indicate that the file
     * has been tampered with.
     */
    if (keychainStatus & (kSecUnlockStateStatus | kSecWritePermStatus)) {
        PM_LOG_ERROR("System roots keychain has invalid permissions: %d", keychainStatus);
        return false;
    }

    CFArrayRef     search_list = CFArrayCreate(kCFAllocatorDefault,
                                               (const void **)&keychain, 1, &kCFTypeArrayCallBacks);
    
    CFMutableDictionaryRef query = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                                             &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFDictionaryAddValue(query, kSecClass, kSecClassCertificate);
    CFDictionaryAddValue(query, kSecMatchLimit, kSecMatchLimitAll);
    CFDictionaryAddValue(query, kSecMatchTrustedOnly, kCFBooleanTrue);
    CFDictionaryAddValue(query, kSecMatchSearchList, search_list);

    CFArrayRef result = nullptr;
    OSStatus statusSecCopy = SecItemCopyMatching(query, (CFTypeRef *)&result);
    auto guardQuery = util::scoped_guard([=]() {
        if (result)
            CFRelease(result);
        if (query)
            CFRelease(query);
        if (search_list)
            CFRelease(search_list);
    });

    if (errSecSuccess != statusSecCopy)
    {
        if (errSecItemNotFound == statusSecCopy)
            PM_LOG_ERROR("No items found in keychain, code %d", statusSecCopy);
        else
            PM_LOG_ERROR("Unable copy certificates from keychain, code %d", statusSecCopy);
        return false;
    }
    
    const auto certCount = CFArrayGetCount(result);
    if (0 == certCount)
    {
        PM_LOG_ERROR("Zero certificates found in keychain copy result");
        return false;
    }

    PM_LOG_INFO("Keychain copy result contains %d certificates", certCount);

    if (!FreeSystemSslCertificates() )
    {
        PM_LOG_ERROR("Failed to FreeSystemSslCertificates");
        return false;
    }

    certificates_.reserve(certCount);
    for(CFIndex crtIndex = 0; crtIndex < certCount; ++crtIndex)
    {
        auto cert = CFArrayGetValueAtIndex(result, crtIndex);
        auto certData = SecCertificateCopyData((SecCertificateRef)cert);
        auto guardData = util::scoped_guard([=]() {
            if (certData)
                CFRelease(certData);
        });
        auto certDataPtr = CFDataGetBytePtr(certData);

        X509* x509Cert = d2i_X509( NULL, ( const unsigned char** )&certDataPtr, CFDataGetLength(certData) );
        if (x509Cert)
            certificates_.push_back(x509Cert);
    }

    PM_LOG_INFO("Internal certificates storage contains %d certificates", certificates_.size());

    return true;
}
bool PmCertRetrieverImpl::FreeSystemSslCertificates()
{
    for (auto cert: certificates_ ) {
        if ( cert )
            X509_free(cert);
    }
    certificates_.clear();
    
    return true;
}

int32_t PmCertRetrieverImpl::GetSslCertificates(std::vector<X509*> &certificates)
{
    int32_t result = -1;

    certificates.clear();
    certificates.reserve(certificates_.size());
        
    for (auto cert: certificates_) {
        if (cert)
            certificates.push_back(X509_dup(cert));
    }
    
    if (!certificates.empty())
        result = 0;
 
    return result;
}

PmCertRetrieverImpl::~PmCertRetrieverImpl()
{
    FreeSystemSslCertificates();
}

}
