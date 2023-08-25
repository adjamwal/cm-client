#include <cinttypes>
#include <xar/xar.h>
#include <openssl/x509.h>
#include <openssl/sha.h>
#include <openssl/err.h>
#include <Security/Security.h>

#include "PmLogger.hpp"
#include "pkg_util.h"

#define CERT_SUBJECT_SF         "Developer ID Installer: Sourcefire Inc. (B74XLY78T6)"
#define CERT_SUBJECT_CISCO_ENT  "Developer ID Installer: Cisco Systems, Inc. (TDNYQP7VRK)"
#define CERT_SUBJECT_CISCO      "Developer ID Installer: Cisco (DE8Y96K9QP)"
#define CERT_ISSUER         "Apple Inc."
#define CERT_AUTH_SUBJECT   "Developer ID Certification Authority"
#define CERT_ROOT           "Apple Root CA"
#define NUM_CERTS           3

typedef enum {
    CODE_SIGN_EUNKNOWN,
    CODE_SIGN_ESTATIC_CODE_NOT_FOUND,
    CODE_SIGN_EREQUIREMENT_FAILURE,
    CODE_SIGN_EFLAG_FAILURE,
    CODE_SIGN_ENOT_SIGNED,
    CODE_SIGN_SIGNED,
} code_sign_result_t;

/**
 * @brief Get issuer name from X509 certificate.
 *
 * @details On success, issuer will be null-terminated.
 *
 * @param[out] issuer - String buffer to store issuer name
 * @param[in] issuer_len - Length of issuer buffer
 * @param[in] certificate_X509 - X509 cerificate
 *
 * @return PKG_SIGN_SUCCESS on success or PKG_SIGN_E* on error
 */
static pkg_sign_error_t
_cert_get_issuer_name(char *issuer, size_t issuer_len, X509 *certificate_X509);

/**
 * @brief Get subject name from X509 certificate.
 *
 * @details On success, subject will be null-terminated.
 *
 * @param[out] subject - String buffer to store subject name
 * @param[in] subject_len - Length of subject buffer
 * @param[in] certificate_X509 - X509 cerificate
 *
 * @return PKG_SIGN_SUCCESS on success or PKG_SIGN_E* on error
 */
static pkg_sign_error_t
_cert_get_subject_name(char *subject, size_t subject_len, X509 *certificate_X509);

/**
 * @brief Creates a certificate object from a DER representation of a
 *        certificate.
 *
 * @details On success, the newly allocated certificate (itemRef) must be
 *          released with CFRelease when you are finished with it.
 *
 * @param[in] x509_der - DER representation of a X.509 certificate
 * @param[out] itemRef - Pointer to newly created certificate object
 *
 * @return 0 on success and itemRef points to a valid certificate object, or -1
 *         on error
 */
static int32_t
_create_cert_ref(CFDataRef x509_der, SecCertificateRef *itemRef);

/**
 * @brief Evaluates trust for a list of certificates based on a given policy.
 *
 * @param[in] certs - List of certificates
 * @param[in] policy_ref - Policy object
 *
 * @return True if trust result type is kSecTrustResultProceed or
 *         kSecTrustResultUnspecified, false otherwise
 */
static bool
_evaluate_cert(CFArrayRef certs, CFTypeRef policy_ref);

/**
 * @brief Verify a certificate chain.
 *
 * @details This method will also return false if an error occurs.
 *
 * @param[in] cert_data_ref - List of DER representation of X.509 certificates
 * @param[in] count - Number of certs in cert_data_ref
 *
 * @return True if certificate chain is valid, false otherwise
 */
static bool
_is_valid_signing(CFDataRef *cert_data_ref, int32_t count);

static pkg_sign_error_t
_cert_get_issuer_name(char *issuer, size_t issuer_len, X509 *certificate_X509)
{
    pkg_sign_error_t ret = PKG_SIGN_EUNKNOWN;
    X509_NAME *issuer_X509_name = NULL;
    X509_NAME_ENTRY *issuer_name_entry = NULL;
    ASN1_STRING *issuer_name_ASN1 = NULL;
    int nid = -1;
    int index = -1;

    if (certificate_X509 == NULL) {
        ret = PKG_SIGN_ENULLARG;
        PM_LOG_ERROR("certificate cannot be null");
        return ret;
    }

    issuer_X509_name = X509_get_issuer_name(certificate_X509);

    if (issuer_X509_name == NULL) {
        PM_LOG_ERROR("issuer name is null. reason: %s\nerror: %s",
                 ERR_reason_error_string(ERR_get_error()), ERR_error_string(ERR_get_error(), NULL));
        return ret;
    }

    nid = OBJ_txt2nid("O"); // organization
    if (nid < 0) {
        PM_LOG_ERROR("invalid orgainzation %d\nreason: %s\nerror: %s",
                 nid, ERR_reason_error_string(ERR_get_error()), ERR_error_string(ERR_get_error(), NULL));
        return ret;
    }

    index = X509_NAME_get_index_by_NID(issuer_X509_name, nid, -1);
    if (index < 0) {
        PM_LOG_ERROR("invalid index %d. reason: %s\nerror: %s", index,
                 ERR_reason_error_string(ERR_get_error()), ERR_error_string(ERR_get_error(), NULL));
        return ret;
    }

    issuer_name_entry = X509_NAME_get_entry(issuer_X509_name, index);
    if (issuer_name_entry == NULL) {
        PM_LOG_ERROR("issuer name is null. reason: %s\nerror: %s",
                 ERR_reason_error_string(ERR_get_error()), ERR_error_string(ERR_get_error(), NULL));
        return ret;
    }

    issuer_name_ASN1 = X509_NAME_ENTRY_get_data(issuer_name_entry);
    if (issuer_name_ASN1 == NULL) {
        PM_LOG_ERROR("failed to convert issuer name. reason: %s\nerror: %s",
                 ERR_reason_error_string(ERR_get_error()), ERR_error_string(ERR_get_error(), NULL));
        return ret;
    }

    const unsigned char *issuer_name = ASN1_STRING_get0_data(issuer_name_ASN1);
    strncpy(issuer, (const char *)issuer_name, issuer_len - 1);
    issuer[issuer_len-1] = '\0';
    ret = PKG_SIGN_SUCCESS;

    return ret;
}

static pkg_sign_error_t
_cert_get_subject_name(char *subject, size_t subject_len, X509 *certificate_X509)
{
    pkg_sign_error_t ret = PKG_SIGN_EUNKNOWN;
    X509_NAME *subject_X509_name = NULL;
    X509_NAME_ENTRY *entry = NULL;
    ASN1_STRING *entry_data = NULL;
    unsigned char *utf8 = NULL;
    int length = 0;
    int idx = -1;

    if (subject == NULL) {
        ret = PKG_SIGN_ENULLARG;
        PM_LOG_ERROR("subject cannot be null");
        goto done;
    }

    if (certificate_X509 == NULL) {
        ret = PKG_SIGN_ENULLARG;
        PM_LOG_ERROR("certificate cannot be null");
        goto done;
    }

    subject_X509_name = X509_get_subject_name(certificate_X509);
    if (subject_X509_name == NULL) {
        PM_LOG_ERROR("subject name returned is null");
        goto done;
    }

    idx = X509_NAME_get_index_by_NID(subject_X509_name, NID_commonName, -1);
    if (idx < 0) {
        PM_LOG_ERROR("name index returned is invalid %d", idx);
        goto done;
    }

    entry = X509_NAME_get_entry(subject_X509_name, idx);
    if (entry == NULL) {
        PM_LOG_ERROR("entry returned is null");
        goto done;
    }

    entry_data = X509_NAME_ENTRY_get_data(entry);
    if (entry_data == NULL) {
        PM_LOG_ERROR("entry data returned is null");
        goto done;
    }

    length = ASN1_STRING_to_UTF8(&utf8, entry_data);
    if (utf8 && (length > 0)) {
        strncpy(subject, (char*)utf8, subject_len - 1);
        subject[subject_len - 1] = '\0';
        ret = PKG_SIGN_SUCCESS;
    } else {
        PM_LOG_ERROR("failed to convert the string to utf8");
        goto done;
    }

done:

    if (utf8) {
        OPENSSL_free( utf8 );
    }

    return ret;
}

static int32_t
_create_cert_ref(CFDataRef x509_der, SecCertificateRef *itemRef)
{
    int32_t ret = -1;

    *itemRef = SecCertificateCreateWithData( NULL, x509_der );

    if (*itemRef) {
        ret = 0;
    } else {
        PM_LOG_ERROR("could not create certificate");
    }

    return ret;
}

static bool
_evaluate_cert(CFArrayRef certs, CFTypeRef policy_ref)
{
    bool trusted = false;
    OSStatus status;
    SecTrustRef trust_ref = NULL;
    CFErrorRef error_ref = NULL;
    const SecTrustOptionFlags flags = (kSecTrustOptionAllowExpired | kSecTrustOptionAllowExpiredRoot);

    status = SecTrustCreateWithCertificates(certs, policy_ref, &trust_ref);
    if (status) {
        PM_LOG_ERROR("Failed to create trust with certificates: %d", status);
        goto done;
    }

    status = SecTrustSetOptions(trust_ref, flags);
    if (status) {
        PM_LOG_ERROR("Failed to set trust options 0x%" PRIx32 ": %d", flags, status);
        goto done;
    }

    if (!SecTrustEvaluateWithError(trust_ref, &error_ref)) {
        PM_LOG_ERROR("Evaluating trust failed: %ld", CFErrorGetCode(error_ref));
        goto done;
    }

    trusted = true;
done:
    if (trust_ref) {
        CFRelease(trust_ref);
        trust_ref = NULL;
    }
    
    if (error_ref) {
        CFRelease(error_ref);
        error_ref = NULL;
    }

    return trusted;
}

static bool _is_valid_signing(CFDataRef *cert_data_ref, int32_t count)
{
    int32_t  i;
    int32_t  cert_count = 0;
    bool     is_valid = false;
    OSStatus status;
    
    SecPolicyRef policy_ref = NULL;
    CFArrayRef cf_certs_array_ref = NULL;
    SecCertificateRef *cert_ref = NULL;
    
    cert_ref = (SecCertificateRef *)calloc(count, sizeof(SecCertificateRef));
    if (!cert_ref) {
        PM_LOG_ERROR("failed to allocate memory for certificate ref");
        goto done;
    }
    
    for (i = 0; i < count; ++i) {
        SecCertificateRef item_ref = NULL;
        
        /* convert data ref to certificate */
        status = _create_cert_ref(cert_data_ref[i], &item_ref);
        
        if (status) {
            goto done;
        }
        
        cert_ref[i] = item_ref;
        cert_count++;
    }
    
    cf_certs_array_ref = CFArrayCreate ((CFAllocatorRef) NULL,
                                        (const void **)cert_ref, cert_count,
                                        &kCFTypeArrayCallBacks);
    
    if (!cf_certs_array_ref) {
        PM_LOG_ERROR("failed allocation of memory for array of CF certificates");
        goto done;
    }
    
    policy_ref = SecPolicyCreateBasicX509();
    if (!policy_ref) {
        PM_LOG_ERROR("Security policy create failed");
        goto done;
    }
    
    is_valid = _evaluate_cert( cf_certs_array_ref,
                              (CFTypeRef)policy_ref );
    
done:
    if (cert_ref) {
        for (i = 0; i < cert_count; ++i) {
            CFRelease(cert_ref[i]);
        }
        free(cert_ref);
        cert_ref = NULL;
    }
    if (cf_certs_array_ref) {
        CFRelease(cf_certs_array_ref);
        cf_certs_array_ref = NULL;
    }
    if (policy_ref) {
        CFRelease(policy_ref);
        policy_ref = NULL;
    }
    
    return is_valid;
}

static pkg_sign_error_t _pkg_verify_p7_sig(const char *package_path,
                                      int sig_idx,
                                      xar_signature_t sig,
                                      X509 ***extracted_certs,
                                      int32_t *num_extracted_certs)
{
    (void)package_path;
    (void)sig_idx;
    uint8_t *plaindata = NULL, *signdata = NULL;
    uint32_t plainlen = 0, signlen = 0;
    CFDataRef *cert_data_ref = NULL;
    STACK_OF(PKCS7_SIGNER_INFO) *si_stack = NULL;
    PKCS7 *p7 = NULL;
    BIO *bio = NULL;
    STACK_OF(X509) *cert_stack = NULL;
    int offs = 0;
    int si_count;
    int i;
    int cert_count;
    X509 **cert_list = NULL;
    pkg_sign_error_t ret = PKG_SIGN_EUNKNOWN;
    bool sig_verified = false;
    unsigned char *tmp_signdata;

    if (xar_signature_copy_signed_data(sig, &plaindata, &plainlen, &signdata, &signlen, NULL) != 0) {
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("Archive %s: signature[%d]: Could not get signed data from XARchive.", package_path, sig_idx);
        goto done;
    }

    // use tmp_signdata as d2i_PKCS7 advances the pointer
    tmp_signdata = signdata;
    p7 = d2i_PKCS7(NULL, (const unsigned char **)&tmp_signdata, signlen);
    if (!p7) {
        ret = PKG_SIGN_EDSIG;
        /* This can fail if the data is not actually a PKCS7 message */
        PM_LOG_INFO("Archive %s: signature[%d]: d2i_PKCS7 failed", package_path, sig_idx);
        goto done;
    }

    bio = PKCS7_dataInit(p7, NULL);
    if (!bio) {
        unsigned long ssl_err = ERR_get_error();
        (void)ssl_err;
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("Archive %s: signature[%d]: PKCS7_dataInit failed: %lu (%s)", package_path, sig_idx, ssl_err, ERR_error_string(ssl_err, NULL));
        goto done;
    }
    while (offs < (int)plainlen) {
        int n = BIO_write(bio, &plaindata[offs], (int)plainlen-offs);
        if (n <= 0) {
            ret = PKG_SIGN_EDSIG;
            PM_LOG_ERROR("Archive %s: signature[%d]: BIO_write failed (%d)", package_path, sig_idx, n);
            goto done;
        }
        offs += n;
    }
    if (!PKCS7_dataFinal(p7, bio)) {
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("Archive %s: signature[%d]: PKCS7_dataFinal failed", package_path, sig_idx);
        goto done;
    }

    if (PKCS7_type_is_signed(p7)) {
        cert_stack = p7->d.sign->cert;
    } else if (PKCS7_type_is_signedAndEnveloped(p7)) {
        cert_stack = p7->d.signed_and_enveloped->cert;
    } else {
        ret = PKG_SIGN_EDSIG;
        PM_LOG_INFO("Archive %s: signature[%d]: Invalid PKCS7 type", package_path, sig_idx);
        goto done;
    }

    cert_data_ref = (CFDataRef *) calloc(sk_X509_num(cert_stack), sizeof(CFDataRef));
    if (!cert_data_ref) {
        ret = PKG_SIGN_EMEM;
        PM_LOG_ERROR("Archive %s: signature[%d]: failed calloc cert array", package_path, sig_idx);
        goto done;
    }

    cert_count = sk_X509_num(cert_stack);
    for (i = 0; i < cert_count; i++) {
        CFDataRef data_ref = NULL;
        unsigned char *der = NULL;
        X509 *cert = sk_X509_value(cert_stack, i);

        int der_len = i2d_X509(cert, &der);
        if (der_len <= 0) {
            ret = PKG_SIGN_EDSIG;
            PM_LOG_ERROR("Archive %s: signature[%d]: i2d_X509 failed (%d)", package_path, sig_idx, der_len);
            goto done;
        }

        data_ref = CFDataCreateWithBytesNoCopy(NULL, der, der_len, kCFAllocatorNull);
        if (!data_ref) {
            ret = PKG_SIGN_EDSIG;
            PM_LOG_ERROR("Archive %s: signature[%d]: could not create data from bytes", package_path, sig_idx);
            goto done;
        }
        cert_data_ref[i] = data_ref;
    }

    if (!_is_valid_signing(cert_data_ref, cert_count)) {
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("Archive %s: signature[%d]: Certificate chain verify failed", package_path, sig_idx);
        goto done;
    }

    si_stack = PKCS7_get_signer_info(p7);
    if (!si_stack) {
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("Archive %s: signature[%d]: No signer", package_path, sig_idx);
        goto done;
    }
    si_count = sk_PKCS7_SIGNER_INFO_num(si_stack);
    for (i = 0; i < si_count; i++)
    {
        PKCS7_SIGNER_INFO *si;
        X509 *x509;
        PKCS7_ISSUER_AND_SERIAL *ias;

        si = sk_PKCS7_SIGNER_INFO_value(si_stack, i);
        ias = si->issuer_and_serial;
        if (!ias) {
            PM_LOG_INFO("Archive %s: signature[%d]: issuer_and_serial not set", package_path, sig_idx);
            continue;
        }
        x509 = X509_find_by_issuer_and_serial(cert_stack, ias->issuer, ias->serial);
        if (!x509) {
            PM_LOG_INFO("Archive %s: signature[%d]: x509 not found", package_path, sig_idx);
            continue;
        }

        if (PKCS7_signatureVerify(bio, p7, si, x509) == 1) {
            sig_verified = true;
            break;
        } else {
            unsigned long ssl_err = ERR_get_error();
            (void)ssl_err;
            PM_LOG_INFO("Archive %s: signature[%d]: PKCS7_signatureVerify failed: %lu (%s)", package_path, sig_idx, ssl_err, ERR_error_string(ssl_err, NULL));
        }
    }

    if (sig_verified) {
        // Copy certs
        X509 *certs_order[NUM_CERTS];
        if (cert_count != NUM_CERTS) {
            ret = PKG_SIGN_EDSIG;
            PM_LOG_ERROR("Archive %s: signature[%d]: unexpected cert chain length %d", package_path, sig_idx, cert_count);
            goto done;
        }
        /* Populate certs in a certain order to accommodate the existing
         * verification code */
        certs_order[0] = sk_X509_value(cert_stack, 2);
        certs_order[1] = sk_X509_value(cert_stack, 0);
        certs_order[2] = sk_X509_value(cert_stack, 1);
        cert_list = (X509 **)calloc(cert_count, sizeof(X509 *));
        if (!cert_list) {
            ret = PKG_SIGN_EMEM;
            PM_LOG_ERROR("Archive %s: signature[%d]: calloc failed", package_path, sig_idx);
            goto done;
        }
        for (i = 0; i < cert_count; i++) {
            X509 *cert = certs_order[i];
            cert_list[i] = X509_dup(cert);
            if (!cert_list[i]) {
                ret = PKG_SIGN_EMEM;
                PM_LOG_ERROR("Archive %s: signature[%d]: X509_dup failed", package_path, sig_idx);
                goto done;
            }
        }

        ret = PKG_SIGN_SUCCESS;
        *extracted_certs = cert_list;
        *num_extracted_certs = cert_count;
        /* hand off */
        cert_list = NULL;
    } else {
        ret = PKG_SIGN_EDSIG;
    }

done:

    if (cert_data_ref) {
        for (i = 0; i < cert_count; ++i) {
            if (cert_data_ref[i]) {
                CFRelease(cert_data_ref[i]);
            }
        }
        free(cert_data_ref);
        cert_data_ref = NULL;
    }

    if (cert_list) {
        for (i = 0; i < cert_count; i++) {
            if (cert_list[i]) {
                X509_free(cert_list[i]);
            }
        }
    }
    free(cert_list);
    cert_list = NULL;

    if (bio) {
        BIO_free(bio);
        bio = NULL;
    }

    if (p7) {
        PKCS7_free(p7);
        p7 = NULL;
    }

    free(plaindata);
    free(signdata);

    return ret;
}

static pkg_sign_error_t _pkg_verify_rsa_sig(const char *package_path,
                                       int sig_idx,
                                       xar_signature_t sig,
                                       X509 ***extracted_certs,
                                       int *num_extracted_certs)
{
    (void)package_path;
    (void)sig_idx;
    pkg_sign_error_t ret = PKG_SIGN_SUCCESS;
    uint8_t *plaindata = NULL, *signdata = NULL;
    uint32_t plainlen = 0, signlen = 0;
    RSA *rsa = NULL;
    EVP_PKEY *pkey = NULL;
    CFDataRef *cert_data_ref = NULL;
    int count;
    int i;
    bool sig_verified = false;
    X509 **certs = NULL;

    // iterate through all certificates associated with that signature
    count = xar_signature_get_x509certificate_count(sig);
    if (count == 0) {
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("Archive %s: signature[%d]: Signature has no certificates.", package_path, sig_idx);
        goto done;
    }

    certs = (X509 **) calloc(count, sizeof(X509 *));
    if (!certs) {
        ret = PKG_SIGN_EMEM;
        PM_LOG_ERROR("Archive %s: signature[%d]: failed calloc cert array", package_path, sig_idx);
        goto done;
    }

    cert_data_ref = (CFDataRef *)calloc(count, sizeof(CFDataRef));
    if (!cert_data_ref) {
        ret = PKG_SIGN_EMEM;
        PM_LOG_ERROR("Archive %s: signature[%d]: failed calloc cert array", package_path, sig_idx);
        goto done;
    }

    /* Create a context object needed for verification.  certs[0] is the
     * certificate to check while certs[1..(count-1)] are the other certificates
     * in the chain. */

    for (i = 0; i < count; i++) {
        const uint8_t *cert_data = NULL;
        uint32_t cert_len = 0;
        CFDataRef data_ref = NULL;
        int32_t err;
        X509 *cert;

        err = xar_signature_get_x509certificate_data(sig, i, &cert_data, &cert_len);
        if (err == -1) {
            ret = PKG_SIGN_EDSIG;
            PM_LOG_ERROR("Archive %s: signature[%d]: could not extract der encoded cert", package_path, sig_idx);
            goto done;
        }

        data_ref = CFDataCreateWithBytesNoCopy(NULL, cert_data, cert_len, kCFAllocatorNull);

        if (!data_ref) {
            ret = PKG_SIGN_EDSIG;
            PM_LOG_ERROR("Archive %s: signature[%d]: could not create data from bytes", package_path, sig_idx);
            goto done;
        }
        cert_data_ref[i] = data_ref;

        cert = d2i_X509(NULL, &cert_data, (int)cert_len);
        if (!cert) {
            ret = PKG_SIGN_EDSIG;
            PM_LOG_ERROR("Archive %s: signature[%d]: could not parse der data", package_path, sig_idx);
            goto done;
        }
        certs[i] = cert;
    }

    if (!_is_valid_signing(cert_data_ref, count)) {
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("Archive %s: signature[%d]: Certificate chain verify failed", package_path, sig_idx);
        goto done;
    }

    if (xar_signature_copy_signed_data(sig, &plaindata, &plainlen, &signdata, &signlen, NULL) != 0) {
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("Archive %s: signature[%d]: Could not get signed data from XARchive.", package_path, sig_idx);
        goto done;
    }

    if (plainlen != 20) { /* SHA1 */
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("Archive %s: signature[%d]: Digest of installer is not SHA1, cannot verify.", package_path, sig_idx);
        goto done;
    }

    pkey = X509_get_pubkey(certs[0]);
    if (!pkey) {
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("Archive %s: signature[%d]: Unable to get pubkey from X509 struct.", package_path, sig_idx);
        goto done;
    }

    if (EVP_PKEY_base_id(pkey) != EVP_PKEY_RSA) {
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("Archive %s: signature[%d]: Public key is not RSA.", package_path, sig_idx);
        goto done;
    }

    rsa = EVP_PKEY_get0_RSA(pkey);
    if (!rsa) {
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("Archive %s: signature[%d]: Could not get RSA data from pkey.", package_path, sig_idx);
        goto done;
    }

    // Verify the signed archive...
    if (RSA_verify(NID_sha1, plaindata, (int)plainlen, (unsigned char *)signdata, signlen, rsa) == 1) {
        sig_verified = true;
    } else {
        unsigned long ssl_err = ERR_get_error();
        (void)ssl_err;
        ret = PKG_SIGN_EDSIG;
        /* This can fail on valid packages built post-Mojave */
        PM_LOG_INFO("Archive %s: signature[%d]: RSA_verify failed: %lu (%s)", package_path, sig_idx, ssl_err, ERR_error_string(ssl_err, NULL));
    }

    if (sig_verified) {
        if (count != NUM_CERTS) {
            ret = PKG_SIGN_EDSIG;
            PM_LOG_ERROR("Archive %s: signature[%d]: unexpected cert chain length %d", package_path, sig_idx, count);
            goto done;
        }
        ret = PKG_SIGN_SUCCESS;
        *extracted_certs = certs;
        *num_extracted_certs = count;
        /* hand off */
        certs = NULL;
    }

done:

    if (certs) {
        for (i = 0; i < count; i++) {
            if (certs[i]) {
                X509_free(certs[i]);
                certs[i] = NULL;
            }
        }
        free(certs);
        certs = NULL;
    }

    if (pkey) {
        EVP_PKEY_free(pkey);
    }

    if (cert_data_ref) {
        for (i = 0; i < count; ++i) {
            if (cert_data_ref[i]) {
                CFRelease(cert_data_ref[i]);
            }
        }
        free(cert_data_ref);
        cert_data_ref = NULL;
    }

    free(plaindata);
    free(signdata);

    return ret;
}

static pkg_sign_error_t _pkg_verify(const char *package_path, bool *has_valid_signature, const char * const trusted_signer)
{
    pkg_sign_error_t ret = PKG_SIGN_EUNKNOWN;
    xar_signature_t sig = NULL;
    xar_t xar_doc = NULL;
    int count = 0;
    int i;
    X509 **certs = NULL;
    char subject[100];
    char issuer[100];
    bool sig_verified = false;
    int sig_idx = 0;

    if (!package_path || !has_valid_signature) {
        ret = PKG_SIGN_ENULLARG;
        goto done;
    }

    if (!*package_path) {
        ret = PKG_SIGN_EARG;
        goto done;
    }

    *has_valid_signature = false;

    // open xar, get signature
    xar_doc = xar_open(package_path, READ);
    if (!xar_doc) {
        ret = PKG_SIGN_EOPEN;
        PM_LOG_ERROR("Could not open %s to extract certificates!", package_path);
        goto done;
    }

    sig = xar_signature_first(xar_doc);
    if (!sig) {
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("No signatures found to extract data from %s.", package_path);
        goto done;
    }
    while (sig) {
        pkg_sign_error_t imn_err;
        imn_err = _pkg_verify_p7_sig(package_path,
                                     sig_idx,
                                     sig,
                                     &certs,
                                     &count);
        if (imn_err == PKG_SIGN_SUCCESS) {
            sig_verified = true;
            break;
        }
        imn_err = _pkg_verify_rsa_sig(package_path,
                                      sig_idx,
                                      sig,
                                      &certs,
                                      &count);
        if (imn_err == PKG_SIGN_SUCCESS) {
            sig_verified = true;
            break;
        }
        sig = xar_signature_next(sig);
        sig_idx++;
    }
    if (!sig_verified) {
        ret = PKG_SIGN_EDSIG;
        PM_LOG_ERROR("Archive %s failed signature verification", package_path);
        goto done;
    }

    /** @note This function verifies that name on the certificate is Cisco, and
     * that the certificate is valid (signed by a trusted CA). It does NOT
     * verify that the signature matches the package contents.
     *
     * Assurance that the signature matches the package contents is provided by
     * the OS when the AMP updater opens the package.
     *
     * The AMP updater also verifies that package file itself matches what is
     * expected by hashing its contents and comparing the hash against a value
     * in a signed XML file (update.xml).
     */

    /* Check the certificate issuer and subject names */

    for (i = 0; i < count; i++) {
        X509* cert = certs[i];

        if (_cert_get_issuer_name(issuer, sizeof(issuer), cert) != PKG_SIGN_SUCCESS) {
            ret = PKG_SIGN_EDSIG;
            PM_LOG_ERROR("Could not get issuer");
            goto done;
        }

        if (_cert_get_subject_name(subject, sizeof(subject), cert) != PKG_SIGN_SUCCESS) {
            ret = PKG_SIGN_EDSIG;
            PM_LOG_ERROR("Could not get issuer");
            goto done;
        }

        switch (i) {
            case 0:
                /* Verify certificate subject is an expected Sourcefire or Cisco
                 * developer team.
                 *
                 * E.g., "Developer ID Installer: Sourcefire Inc. (B74XLY78T6)"
                 */
                if (trusted_signer != NULL) {
                    if (strcmp(subject, trusted_signer) != 0) {
                        ret = PKG_SIGN_EDSIG;
                        PM_LOG_ERROR("Failed to verify subject (%s), expected %s", subject, trusted_signer);
                        goto done;
                    }
                } else if ((strcmp(subject, CERT_SUBJECT_CISCO) != 0) &&
                           (strcmp(subject, CERT_SUBJECT_CISCO_ENT) != 0) &&
                           (strcmp(subject, CERT_SUBJECT_SF) != 0)) {
                    ret = PKG_SIGN_EDSIG;
                    PM_LOG_ERROR("Failed to verify subject (%s)", subject);
                    goto done;
                }

                if (memcmp(issuer, CERT_ISSUER, sizeof(CERT_ISSUER)-1) != 0) {
                    ret = PKG_SIGN_EDSIG;
                    PM_LOG_ERROR("Failed to verify issuer (%s)", issuer);
                    goto done;
                }
                break;
            case 1:
                if (memcmp(CERT_AUTH_SUBJECT, subject, sizeof(CERT_AUTH_SUBJECT)-1) != 0) {
                    ret = PKG_SIGN_EDSIG;
                    PM_LOG_ERROR("Failed to verify subject (%s)", subject);
                    goto done;
                }
                if (memcmp(CERT_ISSUER, issuer, sizeof(CERT_ISSUER)-1) != 0) {
                    ret = PKG_SIGN_EDSIG;
                    PM_LOG_ERROR("Failed to verify issuer (%s)", issuer);
                    goto done;
                }
                break;
            case 2:
                if (memcmp(CERT_ROOT, subject, sizeof(CERT_ROOT)-1) != 0) {
                    ret = PKG_SIGN_EDSIG;
                    PM_LOG_ERROR("Failed to verify subject (%s)", subject);
                    goto done;
                }
                if (memcmp(CERT_ISSUER, issuer, sizeof(CERT_ISSUER)-1) != 0) {
                    ret = PKG_SIGN_EDSIG;
                    PM_LOG_ERROR("Failed to verify issuer (%s)", issuer);
                    goto done;
                }
                break;
            default:
                ret = PKG_SIGN_EDSIG;
                PM_LOG_ERROR("Unexpected cert %s from issuer %s", subject, issuer);
                break;
        }
    }

    ret = PKG_SIGN_SUCCESS;
    *has_valid_signature = true;

done:

    if (certs) {
        for (i = 0; i < count; i++) {
            if (certs[i] != NULL) {
                X509_free(certs[i]);
                certs[i] = NULL;
            }
        }
        free(certs);
        certs = NULL;
    }

    if (xar_doc) {
        xar_close(xar_doc);
    }

    return ret;
}

pkg_sign_error_t pkg_verify(const char *package_path, bool *has_valid_signature)
{
    return _pkg_verify(package_path, has_valid_signature, NULL);
}

pkg_sign_error_t pkg_verify_trusted_signer(const char *package_path, const char * const trusted_signer)
{
    bool has_valid_signature = false;
    pkg_sign_error_t ret = PKG_SIGN_EUNKNOWN;

    do {
        if (trusted_signer == NULL || (trusted_signer[0] == '\0')) {
            /* No signer was specified */
            PM_LOG_ERROR("trusted_signer was not specified (NULL)");
            ret = PKG_SIGN_ENULLARG;
            break;
        }

        ret = _pkg_verify(package_path, &has_valid_signature, trusted_signer);
        if ((ret == PKG_SIGN_SUCCESS) && !has_valid_signature) {
            PM_LOG_ERROR("Package is not signed");
            ret = PKG_SIGN_EDSIG;
            break;
        }
    } while (0);

    return ret;
}

