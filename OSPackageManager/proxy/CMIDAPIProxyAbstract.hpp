#pragma once

#include "cmid/CMIDAPI.h"

class CMIDAPIProxyAbstract {
public:
    virtual ~CMIDAPIProxyAbstract() = default;
    virtual cmid_result_t get_token(char* p_token, int* p_buflen) = 0;
    virtual cmid_result_t get_id(char* p_token, int* p_buflen) = 0;
    virtual cmid_result_t get_url(cmid_url_type_t urlType, char *p_url, int *p_buflen) = 0;
    virtual cmid_result_t refresh_token() = 0;
};
