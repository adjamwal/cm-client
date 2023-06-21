#include "../proxy/CMIDAPIProxyAbstract.hpp"

class CMIDAPIProxy : public CMIDAPIProxyAbstract {
public:
    cmid_result_t get_token(char* p_token, int* p_buflen) override {
        return ::cmid_get_token(p_token, p_buflen);
    }
    cmid_result_t get_id(char* p_token, int* p_buflen) override {
        return ::cmid_get_id(p_token, p_buflen);
    }
    cmid_result_t get_url(cmid_url_type_t urlType, char *p_url, int *p_buflen) override {
        return ::cmid_get_url(urlType, p_url, p_buflen);
    }
    cmid_result_t refresh_token() override {
        return ::cmid_refresh_token();
    }
};
