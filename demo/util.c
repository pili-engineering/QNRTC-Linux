#include "util.h"
#include <assert.h>
#include <string.h>
#include <strings.h>
#include "curl/curl.h"
#include "callback.h"

static size_t WriteBuffer(void *src_, size_t src_size_, size_t blocks_, void *param_)
{
    fprintf(stdout,"%s size:%ld, str: %s\n", __FUNCTION__, src_size_ * blocks_, (char*)src_);
    strncpy((char*)param_, src_, src_size_ * blocks_);
    return src_size_ * blocks_;
}

int get_room_token(const char* app_id, const char* room_id, const char* user_id, char* room_token)
{
    assert(app_id && room_id && user_id && room_token);

    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();

    // set options
    char url_buf[128] = { 0 };
    char* tmp_uid = (char*)user_id;

    if (strncasecmp(tmp_uid, "admin", strlen(tmp_uid)) == 0) {
        snprintf(url_buf,
            sizeof(url_buf),
            "api-demo.qnsdk.com/v1/rtc/token/admin/app/%s/room/%s/user/%s",
            app_id,
            room_id,
            user_id
        );
    } else {
        snprintf(url_buf,
            sizeof(url_buf),
            "api-demo.qnsdk.com/v1/rtc/token/app/%s/room/%s/user/%s",
            app_id,
            room_id,
            user_id
        );
    }

    curl_easy_setopt(curl, CURLOPT_URL, url_buf);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteBuffer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, room_token);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

    // send request now
    int status = 0;
    CURLcode result = curl_easy_perform(curl);
    if (result == CURLE_OK) {
        long code;
        result = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        if (result == CURLE_OK) {
            if (code != 200) {
                status = -2; // server auth failed
            } else {
                status = 0; //success
            }
        } else {
            status = -3; //connect server timeout
        }
    } else {
        status = -3; //connect server timeout
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    fprintf(stdout, "%s http status:%d\n", __FUNCTION__, status);

    return status;
}

void enum_av_device()
{
    // 枚举音频设备
    qn_rtc_enum_audio_devices(0, on_audio_device);
    qn_rtc_enum_audio_devices(1, on_audio_device);

    // 枚举视频设备
    qn_rtc_enum_video_devices(on_video_device);
}

