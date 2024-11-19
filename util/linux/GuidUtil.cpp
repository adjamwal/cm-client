/**
 * @file
 *
 * @copyright (c) 2024 Cisco Systems, Inc. All rights reserved
 */
#include "GuidUtil.hpp"

#include <uuid/uuid.h>

// 36-byte string (plus tailing ’\0’) - CentOS doesn't support uuid_string_t
#define CM_UUID_STR_LEN  36
#define CM_UUID_STR_SIZE (CM_UUID_STR_LEN + 1)

namespace util
{

std::string UTIL_MODULE_API generateGUID()
{
    uuid_t new_uuid;
    char uuid_str[CM_UUID_STR_SIZE] {};
    uuid_generate(new_uuid);
    uuid_unparse(new_uuid, uuid_str);
    return std::string { uuid_str };
}

} //util
