/**
 * @file
 *
 * @copyright (c) 2023 Cisco Systems, Inc. All rights reserved
 */

#pragma once
#include "../proxy/CMIDAPIProxyAbstract.hpp"
#include "gmock/gmock.h"

class MockCMIDAPIProxyAbstract : public CMIDAPIProxyAbstract
{
  public:
    MOCK_METHOD(cmid_result_t, get_token, (char* p_token, int* p_buflen), (override));
    MOCK_METHOD(cmid_result_t, get_id, (char* p_token, int* p_buflen), (override));
    MOCK_METHOD(cmid_result_t, get_url, (cmid_url_type_t urlType, char *p_url, int *p_buflen), (override));
    MOCK_METHOD(cmid_result_t, refresh_token, (), (override));
};
