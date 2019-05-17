#ifndef __CALLBACK_H
#define __CALLBACK_H
#include "qn_event.h"
#include <pthread.h>

// join_room 本地加入房间通知
void on_join_result (int error_code, const char* error_msg, 
    QNRtcUserInfo* user_info_ptr_list[], size_t user_size,
    QNRtcTrackInfo* track_info_ptr_list[],
    size_t track_size, int reconnect_flag);

void on_leave (int error_code_, const char* error_str_, const char* user_id_);

// 房间状态变化通知
void on_room_state_change(QNRtcRoomState state);

// 本地 tracks 发布结果通知
void on_publish_tracks_result(int error_code, const char* error_msg, QNRtcTrackInfo* tracks[], size_t tracks_count);

// 订阅远端 tracks 的结果通知
void on_subscribe_tracks_result (int error_code, const char* error_msg, QNRtcTrackInfo* tracks[], size_t tracks_count);

// 远端发布 tracks 的通知，收到此通知后，本端便可订阅
void on_remote_add_tracks(QNRtcTrackInfo* tracks[], size_t tracks_count);

// 远端取消发布 tracks 的通知，收到此通知后，本端需取消订阅并释放相应的资源
void on_remote_delete_tracks(char* tracks[], size_t tracks_count);

// 远端用户加入房间的通知
void on_remote_user_join (const char* user_id, const char* user_data);

// 远端用户离开房间的通知
void on_remote_user_leave(const char* user_id, int error_code);

// 踢出其它用户的操作结果通知
void on_kickout_result(const char* kicked_out_user_id, int error_code, const char* error_msg);

// 音频 PCM 数据回调接口，本地和远端的均会通过此接口进行通知
void on_audio_frame (
    const unsigned char* audio_data, 
    int bits_per_sample,
    int sample_rate,
    int numbert_of_channels,
    int numbert_of_frames,
    const char* user_id
    );

// 音频设备插拔事件通知
void on_audio_device_state_changed(QNRtcAudioDeviceState device_state, const char* device_guid);

// 当本地开启摄像头或者订阅了远端视频 Track 后，每一帧视频数据均会触发此回调
void on_video_frame(
    const unsigned char* raw_data,
    int data_len,
    int width,
    int height,
    QNRtcVideoCaptureType video_type,
    const char* track_id,
    const char* user_id
);

// 视频帧数据回调，只有调用本地预览接口时触发
void on_preview_video_frame(
    const unsigned char* raw_data,
    int data_len,
    int width,
    int height,
    QNRtcVideoCaptureType video_type
);

// 设备插拔事件通知
void on_video_device_state_changed(
    QNRtcVideoDeviceState device_state,
    const char* device_name
);

// 音频设备枚举回调
void on_audio_device(QNRtcAudioDeviceInfo *device_info);

// 视频设备枚举回调
void on_video_device(QNRtcCameraDeviceInfo *device_info);

// 视频帧解码回调
void on_decode_video_frame(
    const QNRtcEncodedFrame* encoded_frame,
    QNRtcDecodedFrame** decoded_frame
);

// 码率自适应回调
void on_encoder_adjust_setting(
    int bitrate,
    int frame_rate
);

extern void start_publish();
extern void stop_publish();

extern void start_import_thread();
extern void stop_import_thread();

#endif // __CALLBACK_H
