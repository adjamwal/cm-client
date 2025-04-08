/**
 * @file
 * @copyright (c) 2025 Cisco Systems, Inc. All rights reserved
 */

#include "util/ScopedGuard.hpp"
#include "PmCertRetrieverImpl.hpp"
#include "PmLogger.hpp"
#include <string>


namespace PackageManager {
    
/*
** TODO: Need to add support for more linux flavours
*/
#define RHEL_CERT_FILE "/etc/pki/tls/certs/ca-bundle.crt"
#define UBUNTU_CERT_FILE "/etc/ssl/certs/ca-certificates.crt"

bool PmCertRetrieverImpl::LoadSystemSslCertificates() {
    X509_LOOKUP* pLookup = NULL;
    STACK_OF(X509_OBJECT)* pX509Stack = NULL;
    //Init the X509 Store that we will populate
    X509_STORE* pX509Store = X509_STORE_new();
    auto guard = util::scoped_guard([=]() {
        if (pX509Store)
            X509_STORE_free(pX509Store);
    });
    if(NULL == pX509Store) {
        PM_LOG_ERROR("Could not create X509 Store");
        return false;
    }
    //Load cert file; The files are different for RHEL and Ubuntu
    if(0 == X509_STORE_load_locations(pX509Store, RHEL_CERT_FILE , NULL)) { //If the file is not present(perhaps not an RHEL system), we will try to load the Ubuntu file
        //Load the Ubuntu Cert file
        if(0 == X509_STORE_load_locations(pX509Store, UBUNTU_CERT_FILE, NULL)) {
            //Try to load certs from the hardcoded default paths
            if(0 == X509_STORE_set_default_paths(pX509Store)) {
                PM_LOG_ERROR("Could not load cert file into X509 Store"); //Load failed
            }
        }
    }
    //Load additional locations where certs could be present; Certs would have to be present in the form of a hash file with the certname as the file name
    pLookup = X509_STORE_add_lookup(pX509Store, X509_LOOKUP_hash_dir());
    X509_LOOKUP_add_dir(pLookup, "/etc/ssl/certs", X509_FILETYPE_PEM);
    X509_LOOKUP_add_dir(pLookup, "/etc/pki/tls", X509_FILETYPE_PEM);
    X509_LOOKUP_add_dir(pLookup, "/etc/pki/tls/private", X509_FILETYPE_PEM); //Whether these(this & below) directories are required should be verified
    X509_LOOKUP_add_dir(pLookup, "/etc/pki/tls/certs", X509_FILETYPE_PEM);
    
    //Note: The pointer returned by X509_STORE_get0_objects must not be freed by the calling application.
    pX509Stack = X509_STORE_get0_objects(pX509Store);

    if(NULL == pX509Stack) {
        return false;
    }
    for (int iPos = 0; iPos < sk_X509_OBJECT_num(pX509Stack); iPos++) {
        X509_OBJECT* pX509Obj = sk_X509_OBJECT_value(pX509Stack, iPos);
        X509* pX509Cert = X509_OBJECT_get0_X509(pX509Obj);
        if(NULL != pX509Cert) {
            X509_up_ref(pX509Cert);
            certificates_.emplace_back(pX509Cert);
        }
    }

    PM_LOG_INFO("Internal certificates storage contains %d certificates", certificates_.size());
    return  certificates_.size() > 0 ? true : false;
}

bool PmCertRetrieverImpl::FreeSystemSslCertificates() {
    for (auto cert: certificates_ ) {
        if ( cert )
            X509_free(cert);
    }
    certificates_.clear();
    
    return true;
}

int32_t PmCertRetrieverImpl::GetSslCertificates(std::vector<X509*> &certificates) {
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

PmCertRetrieverImpl::~PmCertRetrieverImpl() {
    FreeSystemSslCertificates();
}

}
