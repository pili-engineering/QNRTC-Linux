#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include "util.h"
#include "callback.h"

static char* app_id = "d8lk7l4ed";
static char* room_id = "ggtest";
static char* user_id = "linux_demo";
static char room_token[1024] = {0};

static void *rejoin_thread()
{
    // 字符串必须初始化，否则可能会因结尾字符非 0，而导致 RoomToken 验证异常，导致登录失败
    memset(room_token, 0, sizeof(room_token));
    int repeat_num = 10;
    int result = 0;
    while(repeat_num-- > 0) {
        result = get_room_token(app_id, room_id, user_id, room_token);
        if (0 != result) {
            fprintf(stderr, "get room token failed!\n");
            sleep(1);
            continue;
        } else {
            break;
        }
    }
    if (result != 0) {
        fprintf(stderr, "get room token failed, will exit!\n");
        return NULL;
    }
    // join room, if success auto publish audio & video tracks, and auto subscribe remote tracks
    if(0 != qn_rtc_join_room(room_token, "chengaobao")) {
        rejoin_room();
    }
}

void rejoin_room()
{
    pthread_t thread_handle;
    pthread_create(&thread_handle, NULL, rejoin_thread, NULL);
    pthread_detach(thread_handle);
}

int main(int argc, char const *argv[])
{
    if (argc >= 4) {
        app_id = (char*)argv[1];
        room_id = (char*)argv[2];
        user_id = (char*)argv[3];
    }

    // 枚举音视频设备示例
    // enum_av_device();

    // 回调/事件初始化
    QNRtcEvent rtc_event;
    memset(&rtc_event, 0, sizeof(rtc_event));
    rtc_event.on_join_result = on_join_result;
    rtc_event.on_leave = on_leave;
    rtc_event.on_room_state_change = on_room_state_change;
    rtc_event.on_publish_tracks_result = on_publish_tracks_result;
    rtc_event.on_subscribe_tracks_result = on_subscribe_tracks_result;
    rtc_event.on_remote_add_tracks = on_remote_add_tracks;
    rtc_event.on_remote_delete_tracks = on_remote_delete_tracks;
    rtc_event.on_remote_user_join = on_remote_user_join;
    rtc_event.on_remote_user_leave = on_remote_user_leave;
    rtc_event.on_kickout_result = on_kickout_result;
    rtc_event.on_audio_frame = on_audio_frame;
    rtc_event.on_audio_device_state_changed = on_audio_device_state_changed;
    rtc_event.on_video_frame = on_video_frame;
    rtc_event.on_preview_video_frame = on_preview_video_frame;
    rtc_event.on_video_device_state_changed = on_video_device_state_changed;
    rtc_event.decode_video_frame = on_decode_video_frame;
    rtc_event.on_encoder_adjust_setting = on_encoder_adjust_setting;

    qn_rtc_init(&rtc_event, LOG_INFO, "rtc.log");

    // 获取 room token
    int repeat_num = 10;
    int result = 0;
    while(repeat_num-- > 0) {
        result = get_room_token(app_id, room_id, user_id, room_token);
        if (0 != result) {
            fprintf(stderr,"获取 RoomToken 失败，继续尝试 %d 次!\n", repeat_num);
            sleep(1);
            continue;
        } else {
            break;
        }
    }
    if (result != 0) {
        fprintf(stderr,"获取 RoomToken 失败，程序将退出，请检查您的网络！\n");
        return -1;
    }
    // join room, if success auto publish audio & video tracks, and auto subscribe remote tracks
    qn_rtc_join_room(room_token, "chengaobao");

    fprintf(stdout, "正在加入房间，此后将自动发布音视频 Tracks，并自动订阅远端音视频 Tracks，如需退出，可按回车键！\n");

    // pause
    getchar();

    // release & exit
    stop_publish();    
    stop_import_thread();
    qn_rtc_leave_room();
    qn_rtc_uninit();

    fprintf(stdout, "程序将退出！\n");
    return 0;
}
