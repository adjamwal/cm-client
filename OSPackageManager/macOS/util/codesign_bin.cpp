#include <fcntl.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <Security/Security.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "PmLogger.hpp"
#include "codesign_bin.h"

// mach-o segment,section for timestamp
//
// All of this code should really be in a separate mach.c file
// not splayed out here.
//

#define TIMESTAMP_SEGMENT "__DATA"
#define TIMESTAMP_SECTION "__date"
#define SEGSKIP(x) (x == LC_SEGMENT ? sizeof(struct segment_command) : sizeof(struct segment_command_64) )
#define SECSIZE(x) (x == LC_SEGMENT ? sizeof(struct section) : sizeof(struct section_64) )
#define MHSIZE(x) ( mh.magic == MH_MAGIC_64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header) )

typedef struct segcmd
{
    uint32_t cmd;
    uint32_t cmdsize;
    char segname[16];
} segcmd;

static void * get_arch_offset( void *base )
{
    uint32_t i = 0;
    void *arch_base = base;

    struct fat_header fat_header;
    struct fat_arch fat_arch;

    if ( base == NULL )
    {
        return NULL;
    }

    memcpy( &fat_header, base, sizeof(fat_header) );
    arch_base = static_cast<char*>(arch_base) + sizeof(fat_header);

    for ( i = 0; i < ntohl(fat_header.nfat_arch); i++ )
    {
        cpu_type_t cpu = 0;
        memcpy( &fat_arch,arch_base,sizeof(fat_arch) );
        arch_base = static_cast<char*>(arch_base) + sizeof(fat_arch);

        cpu = ntohl(fat_arch.cputype);

        if ( CPU_TYPE_X86 == cpu || CPU_TYPE_X86_64 == cpu )
        {
            int32_t offset = ntohl(fat_arch.offset);
            arch_base = static_cast<char*>(base) + offset;
            return arch_base;
        }
    }

    return NULL;
}

static void * find_section( void *base, char *sec )
{
    unsigned int i = 0;
    unsigned int nsects = 0;
    unsigned int sectsize = 0;
    unsigned int segskip = 0;

    struct segcmd segcmd;
    
    if ( base == NULL || sec == NULL )
    {
        return NULL;
    }

    memcpy( &segcmd, base, sizeof(segcmd) );

    segskip = SEGSKIP(segcmd.cmd);
    sectsize = SECSIZE(segcmd.cmd);
    nsects = ((segcmd.cmdsize-segskip) / sectsize );

    base = static_cast<char*>(base) + segskip;

    for ( i=0; i < nsects; i++ )
    {
        struct section section;
        memcpy( &section, base, sizeof(section) );

        if ( strcmp( section.sectname, sec ) == 0 )
        {
            return base;
        }

        base = static_cast<char*>(base) + sectsize;
    }

    return NULL;
}

static void * find_segment( void *base, uint32_t ncmds, char *seg )
{
    if ( base == NULL || seg == NULL || ncmds <= 0 )
    {
        return NULL;
    }

    /* try to find section. */

    for (unsigned int i=0; i < ncmds; ++i )
    {
        struct segcmd segcmd;
        memcpy( &segcmd, base, sizeof(segcmd) );
        
        if ( segcmd.cmd == LC_SEGMENT || segcmd.cmd == LC_SEGMENT_64 )
        {
            if ( strcmp( segcmd.segname, seg ) == 0 )
            {
                return base;
            }
        }

        base = static_cast<char*>(base) + segcmd.cmdsize;
    }

    return NULL;
}

static void * find_segment_section( void *base, char *seg, char *sec )
{
    struct mach_header mh;

    void *segment = NULL;

    if ( base == NULL || seg == NULL )
    {
        return NULL;
    }

    memcpy( &mh, base, sizeof(mh) );
    base = static_cast<char*>(base) + MHSIZE(mh.magic);

    segment = find_segment( base, mh.ncmds, seg );

    if ( sec == NULL )
    {
        return segment;
    }

    return find_section( segment, sec );
}

static int find_ts_offset( void *base, uint32_t magic, char *seg, char *sec )
{
    void *section = NULL;

    if ( base == NULL || seg == NULL || sec == NULL )
    {
        return -1;
    }

    if ( (section = find_segment_section( base,seg,sec )) == NULL )
    {
        return -1;
    }

    if ( magic == MH_MAGIC_64 )
    {
        struct section_64 s;
        memcpy( &s, section, sizeof(s) );
        return s.offset;
    }

    else if ( magic == MH_MAGIC )
    {
        struct section s;
        memcpy( &s, section, sizeof(s) );
        return s.offset;
    }

    return -1;
}

static int get_timestamp( const char *path, uint64_t *ts )
{
    int rc = -1;
    int fd = -1;

    struct stat s;
    uint32_t magic = 0;
    uint32_t offset = 0;

    void *addr_base = NULL;
    void *arch_base = NULL;

    if ( path == NULL || ts == NULL )
    {
        return -1;
    }

    if( ( fd = open( path, O_RDONLY ) ) == -1 )
    {
        return -1;
    }

    memset( (void*)&s, 0, sizeof(s) );
    
    if( fstat( fd, &s ) != 0 )
    {
        goto exit_gracefully;
    }

    /* sanity */

    if ( (unsigned long)s.st_size < sizeof(magic) )
    {
        goto exit_gracefully;
    }

    if( ( addr_base = mmap( NULL, (size_t)s.st_size, PROT_READ,
                            MAP_FILE|MAP_SHARED,(int)fd, (off_t)0) ) == MAP_FAILED )
    {
        goto exit_gracefully;
    }

    memcpy( &magic,addr_base,sizeof(magic) );
    arch_base = addr_base;

    /* universal binary, so we need to find our binary. */

    if ( magic == FAT_CIGAM || magic == FAT_MAGIC )
    {
        arch_base = get_arch_offset(addr_base);
    }

    if ( arch_base == NULL )
    {
        goto exit_gracefully;
    }

    /* find ts offset in file */

    offset = find_ts_offset( arch_base, magic, const_cast<char*>(TIMESTAMP_SEGMENT), const_cast<char*>(TIMESTAMP_SECTION) );

    if ( offset > s.st_size )
    {
        goto exit_gracefully;
    }

    /* read ts value. */

    memcpy( ts, (static_cast<char*>(arch_base)+offset), sizeof(uint64_t ) );

    if ( ts != 0 )
    {
        rc = 0;
    }

exit_gracefully:

    munmap( addr_base, s.st_size );
    addr_base = NULL;

    if ( fd >= 0 )
    {
        close(fd);
    }

    return rc;
}

static int get_security_policy(SecPolicyRef* rSecPolicy) {
    if (rSecPolicy == nullptr) {
        return -1;
    }
    
    SecPolicyRef policy = SecPolicyCreateBasicX509();
    if (policy == nullptr) {
        return -1;
    }
    
    *rSecPolicy = policy;
    return 0;
}

/*
 * Use this routine for validating the codesign certificate.
 * Evalualtes the certificate trust.
 * This routine does not check if the certificate is expired.
 * Instead it checks if the second parameter timestamp ts is within the cert expiry period.
 *
 */
static int check_sign_date(SecCertificateRef certRef, const uint64_t ts, CFMutableArrayRef verifyCertList) {
    
    if (certRef == nullptr) {
        return -1;
    }
    
    /* ----- Get a policy ----- */
    SecPolicyRef secPolicy = nullptr;
    int status = get_security_policy(&secPolicy);
    
    if (status < 0) {
        return -1;
    }
    
    SecTrustRef secTrust = nullptr;
    CFMutableArrayRef cfVerifyCertList = nullptr;
    CFMutableArrayRef cfTempVerifyCertList = nullptr;
    SecTrustResultType secTrustResult;

    /* ----- Get trust info ----- */
    if (verifyCertList == nullptr) {
        cfVerifyCertList = CFArrayCreateMutable(nullptr, 0, &kCFTypeArrayCallBacks);
        cfTempVerifyCertList = cfVerifyCertList;
    } else {
        cfTempVerifyCertList = verifyCertList;
    }
    CFArrayInsertValueAtIndex(cfTempVerifyCertList, 0, certRef);
    
    /* ----- Create the SecTrust ----- */
    OSStatus ossRet = SecTrustCreateWithCertificates(cfTempVerifyCertList, secPolicy, &secTrust);
    if (noErr != ossRet) {
        return -1;
    }
    
    CFDateRef cfSignDate = nullptr;
    /* ----- Convert from our uint64_t time to Mac CFDateRef ----- */
    {
        CFAbsoluteTime absTime = ts / 1000.0; // Convert milliseconds to seconds
        cfSignDate = CFDateCreate(nullptr, absTime);
    }
    
    /* ----- Change the date to our sign date ----- */
    ossRet = SecTrustSetVerifyDate(secTrust, cfSignDate);
    if (errSecSuccess != ossRet) {
        return -1;
    }
    
    /* ----- Evaluate trust ----- */
    ossRet = SecTrustEvaluateWithError(secTrust, nullptr);
    if (noErr != ossRet) {
        return -1;
    }
    
    ossRet = SecTrustGetTrustResult(secTrust, &secTrustResult);
    if (noErr != ossRet) {
        return -1;
    }
    
    switch (secTrustResult) {
        case kSecTrustResultUnspecified:
        case kSecTrustResultProceed:
            status = 0;
            break;
            
        case kSecTrustResultRecoverableTrustFailure:
        {
            CFArrayRef trustProperties = SecTrustCopyProperties(secTrust);
            CFIndex numCerts = CFArrayGetCount(trustProperties);
            
            for (CFIndex i = 0; i < numCerts; ++i) {
                CFDictionaryRef certProperties = (CFDictionaryRef)CFArrayGetValueAtIndex(trustProperties, i);
                CFStringRef title = (CFStringRef)CFDictionaryGetValue(certProperties, kSecPropertyTypeTitle);

                if (title == nullptr) {
                    status = -1;
                    break;
                }
            }
            
            CFRelease(trustProperties);
        } break;
            
        case kSecTrustResultDeny:
        case kSecTrustResultInvalid:
        case kSecTrustResultFatalTrustFailure:
        case kSecTrustResultOtherError:
            
        default:
            status = -1;
            break;
    }
    
    // Retrieve custom anchor certificates
    CFArrayRef customAnchors = nullptr;
    ossRet = SecTrustCopyCustomAnchorCertificates(secTrust, &customAnchors);
    if (noErr == ossRet) {
        // Process the custom anchor certificates as needed
        // ...
        CFRelease(customAnchors);
    }
    
    CFRelease(secPolicy);
    CFRelease(secTrust);
    CFRelease(cfVerifyCertList);
    CFRelease(cfSignDate);
    
    return status;
}


static int check_timestamp( const char *path, SecCertificateRef cert_ref, uint64_t killdate )
{
    uint64_t ts = 0;

    if ( path == NULL || cert_ref == NULL )
    {
        PM_LOG_ERROR( "no file specified." );
        return -1;
    }

    if ( get_timestamp( path, &ts ) < 0 )
    {
        PM_LOG_ERROR( "unable to get timestamp out of file (%s)", path);
        return -1;
    }

    if ( ts < killdate )
    {
        PM_LOG_ERROR( "timestamp verification failed, past killdate. (%s)", path );
        return -1;
    }

    if ( check_sign_date( cert_ref, ts, nil) < 0 )
    {
        return -1;
    }

    return 0;
}


static int
copySecStaticCodeFromPath( const char *path, SecStaticCodeRef* pstaticCode)
{
    if (NULL == path || NULL == pstaticCode)
    {
        PM_LOG_DEBUG("invalid input");
        return -1;
    }

    // convert pszFile to CFStringRef
    CFStringRef pathStr = CFStringCreateWithCString(kCFAllocatorDefault,
                                        path,
                                        kCFStringEncodingUTF8 );
    if (pathStr == NULL)
    {
        PM_LOG_DEBUG("CFStringCreateWithCString alloc failure");
        return -1;
    }

    // continue converting pszFile to a CFURLRef
    CFURLRef pathURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                    pathStr,
                                    kCFURLPOSIXPathStyle, false);
    CFRelease(pathStr);
    pathStr = NULL;
    if (pathURL == NULL)
    {
        PM_LOG_DEBUG("CFURLCreateWithString alloc failure");
        return -1;
    }

    // now will determine the SecStaticCodeRef for pszFile
    OSStatus retOSStatus = SecStaticCodeCreateWithPath(pathURL,
                                              kSecCSDefaultFlags,
                                              pstaticCode);
    CFRelease(pathURL);
    pathURL = NULL;
    if (errSecSuccess != retOSStatus)
    {
        PM_LOG_DEBUG( "SecStaticCodeCreateWithPath %d", retOSStatus);
        return -1;
    }

    return 0;
}

int check_signature( const char* path)
{
    int rc = -1;

    OSStatus retOSStatus = -1;
    SecStaticCodeRef codeRef = nil;

    if (NULL == path)
    {
        PM_LOG_DEBUG("invalid input");
        return -1;
    }

    if( copySecStaticCodeFromPath( path, &codeRef ) < 0 )
    {
        goto done;
    }

    // Mac Api to test if the binary was signed by codesign and
    // it has a valid cert
    retOSStatus = SecStaticCodeCheckValidity(codeRef,
                                             kSecCSDefaultFlags,
                                             NULL);
    if (errSecSuccess != retOSStatus)
    {
        PM_LOG_DEBUG("[%s] fails signature check", path);
        goto done;
    }

    rc = 0;

 done:

    if( nil != codeRef )
    {
        CFRelease(codeRef);
        codeRef = nil;
    }

    return rc;
}

int check_signer( const char* path, const char* signer, SigType sig_type, uint64_t killdate )
{
    int rc = -1;

    OSStatus retOSStatus = -1;
    SecCSFlags flags = kSecCSInternalInformation
        | kSecCSSigningInformation
        | kSecCSRequirementInformation;

    SecStaticCodeRef codeRef = nil;
    CFDictionaryRef dicRef = nil;

    if( NULL == path  || NULL == signer )
    {
        PM_LOG_DEBUG( "invalid input" );
        return rc;
    }

    if( SigType::SIGTYPE_NATIVE != sig_type )
    {
        PM_LOG_DEBUG( "unsupported signature type" );
        return rc;
    }

    // private function to get SecStaticCodeRef
    if( copySecStaticCodeFromPath(path, &codeRef) < 0 )
    {
        goto done;
    }

    // This will extract the signing information, ie cert
    retOSStatus = SecCodeCopySigningInformation(codeRef, flags, &dicRef);
    if (errSecSuccess != retOSStatus || dicRef == nil )
    {
        PM_LOG_DEBUG("SecCodeCopySigningInformation could not extract cert data");
        goto done;
    }

    if (0 == strcmp(signer, SIGNER_CISCO_TEAMID))
    {
        CFStringRef cfstrTeamID = (CFStringRef)CFDictionaryGetValue(dicRef, kSecCodeInfoTeamIdentifier);
        char aszSignTeamID[32] = "";
        if (NULL == cfstrTeamID || !CFStringGetCString(cfstrTeamID, aszSignTeamID, sizeof(aszSignTeamID), kCFStringEncodingMacRoman))
        {
            PM_LOG_ERROR("Failed to extract team ID from signature");
            goto done;
        }
        if (0 != strcmp(signer, aszSignTeamID))
        {
            PM_LOG_ERROR("Mismatched Team ID. Expected: %s, Found: %s", signer, aszSignTeamID);
            goto done;
        }
        rc = 0;
        CFDateRef cfdSignTimestamp = (CFDateRef)CFDictionaryGetValue(dicRef, kSecCodeInfoTimestamp);
        if (nil != cfdSignTimestamp)
        {
            CFAbsoluteTime absTimestamp = CFDateGetAbsoluteTime(cfdSignTimestamp);
            if (absTimestamp + kCFAbsoluteTimeIntervalSince1970 < (CFAbsoluteTime)killdate)
            {
                PM_LOG_ERROR("timestamp verification failed, past killdate. (%s)", path);
                rc = -1;
            }
        }
    }
    else
    {
        // we need to extract the cert information so we can look at all of them
        CFArrayRef certArrayRef = (CFArrayRef)CFDictionaryGetValue(dicRef,
                                                        kSecCodeInfoCertificates);
        CFIndex n, countIndex;
        if (NULL == certArrayRef)
        {
            goto done;
        }

        countIndex = CFArrayGetCount(certArrayRef);
        for (n = 0; n < countIndex; n++ )
        {
            SecCertificateRef certRef = (SecCertificateRef)CFArrayGetValueAtIndex(certArrayRef, n);
            if( NULL == certRef )
            {
                continue;
            }
            CFStringRef commonName = nil;
            retOSStatus = SecCertificateCopyCommonName(certRef, &commonName);
            if (errSecSuccess == retOSStatus && nil != commonName )
            {
                char common_name[200] = "";
                if(CFStringGetCString(commonName,
                                      common_name,
                                      sizeof( common_name ),
                                      kCFStringEncodingMacRoman) )
                {
                    if (0 == strcmp(signer, common_name))
                    {
                        rc = 0;
                        if (0 == strcmp(signer, SIGNER_CISCO_CN))
                        {
                            rc = check_timestamp(path, certRef, killdate);
                        }
                    }
                }

                if ( commonName != nil )
                {
                    CFRelease(commonName);
                    commonName = nil;
                }
            }
        }
    }

    if( rc < 0 )
    {
        PM_LOG_DEBUG("signer comparison failed" );
    }
        
 done:

    if(dicRef != nil)
    {
        CFRelease(dicRef);
        dicRef = nil;
    }

    if(codeRef != nil)
    {
        CFRelease(codeRef);
        codeRef = nil;
    }

    return rc;
}
