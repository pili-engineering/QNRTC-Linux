#include "callback.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include "qn_room.h"
#include "util.h"

static const unsigned char nal_header[3] = {0x00, 0x00, 0x01};
static const int header_length = 3;
static char** local_tracks_id = NULL;
static size_t local_tracks_count = 0;
static pthread_t import_work_thread = 0;
static char exit_flag = 0;

static void* import_h264_video_frame_thread(void*);

// join_room 本地加入房间通知
void on_join_result (int error_code, const char* error_msg, 
    QNRtcUserInfo* user_info_ptr_list[], size_t user_size,
    QNRtcTrackInfo* track_info_ptr_list[],
    size_t track_size, int reconnect_flag)
{
    fprintf(stdout, "%s error code:%d, error_msg:%s, user_size:%lu, track_size:%lu, reconn flag:%s\n", 
        __FUNCTION__,
        error_code,
        error_msg,
        user_size,
        track_size,
        reconnect_flag ? "true" : "false"
    );
    if (error_code != 0) {
        if (error_code == Err_License_expired) {
            fprintf(stderr, "加入房间失败， 错误码:%d, license 已过期！\n", error_code);
            return;
        }
        fprintf(stderr, "加入房间失败， 错误码:%d, 自动登陆中...\n", error_code);
        // auto rejoin
        rejoin_room();
        return;
    }
    fprintf(stdout, "加入房间成功!\n");
    if (track_size > 0) {
        // 由 on_remote_add_tracks 接口通知, 并由其决定是否订阅
        on_remote_add_tracks(track_info_ptr_list, track_size);
    }
    if (!reconnect_flag) {
        // 自动发布
        start_publish();
    }
}

void on_leave(int error_code, const char* error_str, const char* user_id)
{
    fprintf(stderr, "您以非正常方式离开了房间，错误码：%d, 错误信息：%s, 踢出者:%s\n",
        error_code,
        error_str,
        user_id
    );
}

// 房间状态变化通知
void on_room_state_change(QNRtcRoomState state)
{
    fprintf(stderr, "网络断开，自动登陆中...\n");
    // must stop import h264 frame thread
    stop_import_thread();
}

// 本地 tracks 发布结果通知
void on_publish_tracks_result(int error_code, const char* error_msg, QNRtcTrackInfo* tracks[], size_t tracks_count)
{
    if (error_code != 0) {
        fprintf(stderr, "发布失败，错误码：%d，错误信息：%s\n", error_code, error_msg);
        return;
    } else {
        fprintf(stdout, "发布成功，Track 数量：%lu\n", tracks_count);
    }
    // record local tracks id
    if (local_tracks_count > 0) {
        size_t i = 0;
        for(; i < local_tracks_count; i++) {
            free(local_tracks_id[i]);
        }
        free(local_tracks_id);
        
    }
    local_tracks_count = tracks_count;
    local_tracks_id = malloc(local_tracks_count * sizeof(char*));
    size_t i = 0;
    for(; i < tracks_count; ++i) {
        fprintf(stdout, "%s track id:%s, kind::%s, tag:%s, width:%d, height:%d\n", 
            __FUNCTION__, 
            tracks[i]->track_id, 
            tracks[i]->kind, 
            tracks[i]->tag,
            tracks[i]->width,
            tracks[i]->height
            );
        local_tracks_id[i] = (char*)malloc(strlen(tracks[i]->track_id) + 1);
        memset(local_tracks_id[i], 0, strlen(tracks[i]->track_id));
        memcpy(local_tracks_id[i], tracks[i]->track_id, strlen(tracks[i]->track_id));
    }
    // start import h.264 nalus
    start_import_thread();
}   

// 订阅远端 tracks 的结果通知
void on_subscribe_tracks_result (int error_code, const char* error_msg, QNRtcTrackInfo* tracks[], size_t tracks_count)
{
    if (error_code != 0) {
        fprintf(stderr, "订阅失败，错误码：%d，错误信息：%s\n", error_code, error_msg);
        return;
    } else {
        fprintf(stdout, "订阅成功，Track 数量：%ld\n", tracks_count);
    }
    size_t i = 0;
    for(; i < tracks_count; ++i) {
        fprintf(stdout, "%s track id:%s, kind::%s, tag:%s\n", 
            __FUNCTION__, 
            tracks[i]->track_id, 
            tracks[i]->kind, 
            tracks[i]->tag
            );
    }
}

// 远端发布 tracks 的通知，收到此通知后，本端便可订阅
void on_remote_add_tracks(QNRtcTrackInfo* tracks[], size_t tracks_count)
{
    fprintf(stdout, "远端发布了 Tracks， count:%ld，本端将自动订阅。\n", tracks_count);
    // Demo 上演示为自动订阅，用户可根据场景自行决定订阅时机
    int ret = qn_rtc_subscribe(tracks, tracks_count);
    if (0 != ret) {
        fprintf(stderr, "订阅失败！ qn_rtc_subscribe failed, return:%d\n", ret);
    }
}

// 远端取消发布 tracks 的通知，收到此通知后，本端需取消订阅并释放相应的资源
void on_remote_delete_tracks(char* tracks[], size_t tracks_count)
{
    fprintf(stdout, "远端取消发布了 Tracks， count:%ld\n", tracks_count);
    qn_rtc_unsubscribe(tracks, tracks_count);
}

// 远端用户加入房间的通知
void on_remote_user_join (const char* user_id, const char* user_data)
{
    fprintf(stdout, "新用户加入房间 user id:%s, user data:%s\n",
        user_id,
        user_data
    );
}

// 远端用户离开房间的通知
void on_remote_user_leave(const char* user_id, int error_code)
{
    fprintf(stdout, "用户离开房间 user id:%s, error code:%d\n", 
        user_id,
        error_code
    );
}

// 踢出其它用户的操作结果通知
void on_kickout_result(const char* kicked_out_user_id, int error_code, const char* error_msg)
{
    fprintf(stdout, "踢出远端用户回调， kicked out user id:%s, error code:%d, error msg:%s\n", 
        kicked_out_user_id,
        error_code,
        error_msg
    );
}

// 音频 PCM 数据回调接口，本地和远端的均会通过此接口进行通知
void on_audio_frame (
    const unsigned char* audio_data, 
    int bits_per_sample,
    int sample_rate,
    int numbert_of_channels,
    int numbert_of_frames,
    const char* user_id)
{

}

// 音频设备插拔事件通知
void on_audio_device_state_changed(QNRtcAudioDeviceState device_state, const char* device_guid)
{
    fprintf(stdout, "%s device guid:%s, device_state:%d\n", 
        __FUNCTION__,
        device_guid,
        (int)device_state
    );
}

// 当本地开启摄像头或者订阅了远端视频 Track 后，每一帧视频数据均会触发此回调
void on_video_frame(
    const unsigned char* raw_data,
    int data_len,
    int width,
    int height,
    QNRtcVideoCaptureType video_type,
    const char* track_id,
    const char* user_id)
{
    fprintf(stdout, "%s user id:%s, track id:%s, %dx%d\n", 
        __FUNCTION__,
        user_id,
        track_id,
        width,
        height
    );
}

// 视频帧数据回调，只有调用本地预览接口时触发
void on_preview_video_frame(
    const unsigned char* raw_data,
    int data_len,
    int width,
    int height,
    QNRtcVideoCaptureType video_type)
{

}    

// 设备插拔事件通知
void on_video_device_state_changed(
    QNRtcVideoDeviceState device_state,
    const char* device_name)
{
    fprintf(stdout, "%s device name:%s, device state:%d\n", 
        __FUNCTION__,
        device_name,
        (int)device_state
    );
}

void on_audio_device(QNRtcAudioDeviceInfo *device_info)
{
    assert(NULL != device_info);
    fprintf(stdout, 
        "device name:%s, device id:%s, is default:%s, device index:%d\n",
        device_info->device_id,
        device_info->device_name,
        device_info->is_default?"true":"false",
        device_info->device_index
    );
}

void on_video_device(QNRtcCameraDeviceInfo *device_info)
{
    assert(NULL != device_info);
    fprintf(stdout, 
        "camera device id:%s, device name:%s, capbility count:%d\n",
        device_info->device_id,
        device_info->device_name,
        device_info->capability_count
    );
}

void on_decode_video_frame(const QNRtcEncodedFrame* encoded_frame, QNRtcDecodedFrame** decoded_frame)
{
    assert(encoded_frame);
    // fprintf(stdout, "%s Received video encoded frame, timestamp:%ulld\n", __FUNCTION__, encoded_frame->time_stamp);
}

void on_encoder_adjust_setting(int bitrate, int frame_rate)
{
    fprintf(stdout, "编码器参数调整： new targe bitrate:%d, frame_rate:%d\n", bitrate, frame_rate);
}

// work thread of import external video h264 nalus
void* import_h264_video_frame_thread(void *arg)
{
    exit_flag = 0;
    FILE* fp = fopen("./external.h264", "rb");
    if (!fp) {
        fprintf(stderr, "请检查本地文件：external.h264 文件是否存在，确认后再次启动程序！\n");
        return NULL;
    }
    assert(fp);
    unsigned char *src_buf = (unsigned char*)malloc(1024 * 1024);
    assert(src_buf);
    memset(src_buf, 0, 1024 * 1024);
    int src_len = 0;
    size_t read_len = 0;
    size_t read_count = 0;
    char first_flag = 1;
    unsigned char is_key_frame = 0;
    int read_byte;
    int payload_index = 0;
    int last_payload_pos = 0;
    int flag = 0;

    const int max_nalus_count = 4;
    int nal_index = -1;
    QNRtcH264Nal **nalus_array = (QNRtcH264Nal **)malloc(sizeof(QNRtcH264Nal *) * max_nalus_count);
    while(++nal_index < max_nalus_count) {
        nalus_array[nal_index] = (QNRtcH264Nal*)malloc(sizeof(QNRtcH264Nal));
    }
    
    struct timespec ts;
    while (!exit_flag) {
        read_count = 0;
        first_flag = 1;
        is_key_frame = 0;
        payload_index = 0;
        last_payload_pos = 0;
        while((read_byte = fgetc(fp)) != EOF){
            src_buf[read_count] = (unsigned char)read_byte;
            ++read_count;
            if (read_count >= header_length
                && memcmp(src_buf + read_count - header_length, nal_header, header_length) == 0) {
            
                if(first_flag) {
                    first_flag = 0;
                    continue;
                }
                // read next byte
                read_byte = fgetc(fp);
                src_buf[read_count] = (unsigned char)read_byte;
                ++read_count;

                // 4 = header_length + 1;
                if ((*(src_buf + last_payload_pos + header_length) == 0x67 
                    || *(src_buf + last_payload_pos + header_length) == 0x68)) {
                    nalus_array[payload_index]->payload_size = 
                        read_count - 4 - last_payload_pos;

                    nalus_array[payload_index]->payload = 
                        src_buf + last_payload_pos;
                    
                    ++payload_index;
                    last_payload_pos = read_count - 4;
                    continue;
                } else if ((*(src_buf + last_payload_pos + header_length) == 0x65)) {
                    is_key_frame = 1;

                    nalus_array[payload_index]->payload_size = 
                    read_count - 4 - last_payload_pos;

                    nalus_array[payload_index]->payload = 
                    src_buf + last_payload_pos;
                    ++payload_index;
                    last_payload_pos = read_count - 4;
                } else {
                    nalus_array[payload_index]->payload_size = 
                    read_count - 4 - last_payload_pos;

                    nalus_array[payload_index]->payload = 
                    src_buf + last_payload_pos;
                    ++payload_index;
                    last_payload_pos = read_count - 4;
                }

                // // if (!is_key_frame) {
                //     for(size_t i = 0; i < payload_index; i++)
                //     {
                //         fprintf(stdout, "payload index:%d, size:%d\n", i, nalus_array[i]->payload_size);
                //         for(size_t j = 0; j < (nalus_array[i]->payload_size >= 100 ? 100 : nalus_array[i]->payload_size); j++)
                //         {
                //             fprintf(stdout, "%02x ", nalus_array[i]->payload[j]);
                //         }
                //         fprintf(stdout, "\n");
                //     }
                // // }
                // fflush(stdout);
                qn_rtc_input_h264_frame(nalus_array, payload_index, is_key_frame);

                ts.tv_sec = 0;
                ts.tv_nsec = 33 * 1000 * 1000;
                nanosleep(&ts, NULL);

                fseek(fp, -4, SEEK_CUR);
                
                break;
            }
        }
        if (read_byte == EOF) {
            fseek(fp, 0, SEEK_SET);
        }
        continue;
    }
    nal_index = -1;
    while(++nal_index < max_nalus_count) {
        free(nalus_array[nal_index]);
    }
    free(nalus_array);
    fclose(fp);
    free(src_buf);
}

void start_publish()
{
    QNRtcTrackInfo* track_info_list[2];
    QNRtcTrackInfo audio_track, video_track;
    memset(&audio_track, 0, sizeof(audio_track));
    memset(&video_track, 0, sizeof(video_track));

    int track_count = 0;
    audio_track.kind = KIND_AUDIO;
    audio_track.tag = "microphone";
    audio_track.is_master = 1;
    audio_track.is_muted = 0;
    audio_track.max_bitrate = 64 * 1024;
    track_info_list[track_count] = &audio_track;
    track_count++;

    video_track.camera_device_id = "";
    video_track.kind = KIND_VIDEO;
    video_track.tag = "external";
    video_track.is_master = 1;
    video_track.is_muted = 0;
    video_track.width = 540;                // only test
    video_track.height = 960;               // only for test
    video_track.max_fps = 30;               // only for test
    video_track.max_bitrate = 2000 * 1024;  // only for test
    video_track.source_type = tst_ExternalYUV;
    track_info_list[track_count] = &video_track;
    track_count++;

    int ret = qn_rtc_publish(track_info_list, track_count);
    if (ret != 0) {
        fprintf(stderr, "%s qn_rtc_publish failed, error code:%d\n", __FUNCTION__, ret);
    }
}

void stop_publish()
{
    if (local_tracks_count <= 0 || !local_tracks_id) {
        return;
    }
    fprintf(stdout, "%s unpublish result:%d\n", 
        __FUNCTION__, 
        qn_rtc_unpublish(local_tracks_id, local_tracks_count)
    );
    size_t i = 0;
    for (; i < local_tracks_count; ++i) {
        free(local_tracks_id[i]);
    }
    free(local_tracks_id);
}

void start_import_thread()
{
    stop_import_thread();
    int ret = pthread_create(&import_work_thread, NULL, &import_h264_video_frame_thread, NULL);
    if (ret != 0) {
        fprintf(stderr, "%s pthread_create failed, error:%d\n", __FUNCTION__, ret);
    }
    pthread_setname_np(import_work_thread, "import_h264_thread");
}

void stop_import_thread()
{
    if (0 == import_work_thread) {
        return;
    }
    int result = pthread_kill(import_work_thread, 0);
    if (ESRCH == result) {
        return;
    }
    exit_flag = 1;
    pthread_join(import_work_thread, NULL);
    import_work_thread = 0;
}

