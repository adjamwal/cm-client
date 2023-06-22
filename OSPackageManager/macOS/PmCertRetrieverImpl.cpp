/**
 * @file
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#include "PmCertRetrieverImpl.hpp"

#include <CoreFoundation/CFArray.h>
#include <Security/Security.h>


namespace PackageManager
{
bool PmCertRetrieverImpl::LoadSystemSslCertificates()
{
    CFMutableDictionaryRef query = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                                             &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFDictionaryAddValue(query, kSecClass, kSecClassCertificate);
    CFDictionaryAddValue(query, kSecMatchLimit, kSecMatchLimitAll);

    //Do we need to delete the result's contents?
    CFArrayRef result = nullptr;
    OSStatus statusSecCopy = SecItemCopyMatching(query, (CFTypeRef *)&result);
    CFRelease(query);
    
    if (errSecSuccess != statusSecCopy)
        return false;
    
    //TODO: Add logging here, when it is ready
    const auto certCount = CFArrayGetCount(result);
    if (!FreeSystemSslCertificates() || 0 == certCount)
        return false;
    
    certificates_.reserve(certCount);
    for(CFIndex crtIndex = 0; crtIndex < certCount; ++crtIndex)
    {
        auto cert = CFArrayGetValueAtIndex(result, crtIndex);
        auto certData = SecCertificateCopyData((SecCertificateRef)cert);
        auto certDataPtr = CFDataGetBytePtr(certData);

        X509* x509Cert = d2i_X509( NULL, ( const unsigned char** )&certDataPtr, CFDataGetLength(certData) );
        if (x509Cert)
            certificates_.push_back(x509Cert);
        
        CFRelease(certData);
    }
    CFRelease(result);
    
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
