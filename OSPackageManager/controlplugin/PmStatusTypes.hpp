#pragma once

enum class PM_STATUS {
    PM_OK = 0,
    PM_FAIL = -1,	   /* Operation failed */
    PM_FAIL_CERT = -2, /* Cert Security Verification Error */
    PM_ERROR = -3,     /* General error */
    PM_NOENT = -4,	   /* No such entry */
    PM_EXISTS = -5,	   /* Entry already exists */
    PM_FULL = -6,	   /* Table is full */
    PM_PERM = -7,	   /* Permission denied */
    PM_INVAL = -8,	   /* Invalid parameter */
    PM_NOMEM = -9,	   /* Cannot allocate necessary memory */
    PM_NO_INIT = -10,   /* Not Initialised*/
    PM_CLOUD_ERR = -11,   /* Error response from cloud*/
    PM_CLOUD_RATE_LIMIT = -12, /*Rate limit response from cloud*/
    PM_TIMEOUT = -13, /* Time out*/
    PM_CODE_SIGN_EXPIRED = -14,  /* Codesign time expired */
    PM_CODE_SIGNER_MISMATCH = -15, /* Untrusted signer disallowed */
    PM_CODE_SIGN_VERIFICATION_FAILED = -16, /* Codesign verification failure */
    PM_INSUFFICIENT_BUFFER = -17, /* Insufficient buffer */
    PM_MAX = -18	   /* Largest error number */
};
