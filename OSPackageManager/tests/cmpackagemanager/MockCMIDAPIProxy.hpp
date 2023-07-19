
#pragma once
#include "gmock/gmock.h"

class FakeCMIDAPIProxy : public CMIDAPIProxyAbstract {
public:
    cmid_result_t get_token(char* p_token, int* p_buflen) {
        if (!initialised_)
            return CMID_RES_NOT_INITED;

        const auto cmid = "cmid_token";
        if (!p_buflen)
            return CMID_RES_INVALID_ARG;
        
        if (!p_token || *p_buflen < strlen(cmid)) {
            *p_buflen = strlen(cmid) + 1;
            return CMID_RES_INSUFFICIENT_LEN;
        }
        
        std::strncpy(p_token, cmid, *p_buflen);
        return CMID_RES_SUCCESS;
    }
    cmid_result_t get_id(char* p_id, int* p_buflen) {
        
        if (!initialised_)
            return CMID_RES_NOT_INITED;
        
        const auto cmid = "cmid_identity";
        if (!p_buflen)
            return CMID_RES_INVALID_ARG;
        
        if (!p_id || *p_buflen < strlen(cmid)) {
            *p_buflen = strlen(cmid) + 1;
            return CMID_RES_INSUFFICIENT_LEN;
        }
        
        std::strncpy(p_id, cmid, *p_buflen);
        return CMID_RES_SUCCESS;
    }
    
    cmid_result_t get_url(cmid_url_type_t urlType, char *p_url, int *p_buflen) {
        if (!initialised_)
            return CMID_RES_NOT_INITED;
        
        const auto get_url = [](cmid_url_type_t urlType) {
            switch (urlType) {
                case CMID_EVENT_URL: return "cmid_event_url";
                case CMID_CATALOG_URL: return "cmid_catalog_url";
                case CMID_CHECKIN_URL: return "cmid_checkin_url";
            }
        };

        if (!p_buflen)
            return CMID_RES_INVALID_ARG;
        
        const auto cmidUrl = get_url(urlType);
        
        if (!p_url || *p_buflen < strlen(cmidUrl)) {
            *p_buflen = (int)strlen(cmidUrl) + 1;
            return CMID_RES_INSUFFICIENT_LEN;
        }
        
        std::strncpy(p_url, cmidUrl, *p_buflen);
        return CMID_RES_SUCCESS;
    }
    
    cmid_result_t refresh_token() {
        if (!initialised_)
            return CMID_RES_NOT_INITED;

        return CMID_RES_SUCCESS;
    }
    
    FakeCMIDAPIProxy(bool value) {
        initialised_ = value;
    }
    
private:
    bool initialised_{false};
};

class MockCMIDAPIProxy : public CMIDAPIProxyAbstract {
public:
    MockCMIDAPIProxy(bool initialised) : fake_(initialised) {}
    
    MOCK_METHOD(cmid_result_t, get_token, (char*, int*));
    MOCK_METHOD(cmid_result_t, get_id, (char*, int*));
    MOCK_METHOD(cmid_result_t, get_url, (cmid_url_type_t, char*, int* ));
    MOCK_METHOD(cmid_result_t, refresh_token, ());

    // Delegates the default actions of the methods to a FakeCMIDAPIProxy object.
    // This must be called *before* the custom ON_CALL() statements.
    void DelegateToFake() {
        ON_CALL(*this, get_token).WillByDefault([this](char* s, int* n) {
            return fake_.get_token(s, n);
        });
        ON_CALL(*this, get_id).WillByDefault([this](char* s, int* n) {
            return fake_.get_id(s, n);
        });
        ON_CALL(*this, get_url).WillByDefault([this](cmid_url_type_t t, char *s, int *n) {
            return fake_.get_url(t, s, n);
        });
        ON_CALL(*this, refresh_token).WillByDefault([this]() {
            return fake_.refresh_token();
        });
    }
    
private:
    FakeCMIDAPIProxy fake_;
};
