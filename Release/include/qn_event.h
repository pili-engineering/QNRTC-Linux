#ifndef __QN_EVENT_H__
#define __QN_EVENT_H__
#include "qn_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// 音频设备枚举列表由此接口通知给开发者
// @param device_info 音频设备属性 
typedef void (QN_API * on_audio_device_enum)(QNRtcAudioDeviceInfo *device_info);

// 视频设备枚举列表由此接口通知给开发者
// @param device_info 视频设备属性 
typedef void (QN_API * on_camera_device_enum)(QNRtcCameraDeviceInfo *device_info);

typedef struct
{
    // join_room 本地加入房间通知
    // @param error_code 0:success, failed: error code
    // @param error_msg 当 error_code 非 0 时，表示错误描述信息
    // @param user_info_ptr_list 用户信息列表, QNRtcUserInfo 指针数组，每个元素表示一个 QNRtcUserInfo 元素指针
    // @param user_size 用户数量，即 user_info_ptr_list 元素个数
    // @param track_info_ptr_list 用户信息列表, QNRtcTrackInfo 指针数组，每个元素表示一个 QNRtcTrackInfo 元素指针
    // @param track_size 用户数量，即 track_info_ptr_list 元素个数
    // @param reconnect_flag 标识是否是重连的回调 
    void (QN_API * on_join_result) (
        int error_code, 
        const char* error_msg, 
        QNRtcUserInfo* user_info_ptr_list[], 
        size_t user_size, 
        QNRtcTrackInfo* track_info_ptr_list[], 
        size_t track_size, 
        int reconnect_flag
    );

    // 只有当自己以非正常方式离开的时候才会触发，如异地登录、被踢出房间
    // @param error_code_ 离开房间的错误码
    // @param error_str_ 错误码描述字符串
    // @param user_id_ 仅用于当被踢出房间时，传递操作者的 User Id
    void (QN_API * on_leave) (int error_code_, const char* error_str_, const char* user_id_);

    // 房间状态变化通知
    // @param state 当前房间状态 
    void (QN_API * on_room_state_change) (QNRtcRoomState state);

    // 本地 tracks 发布结果通知
    // @param error_code 0:success, failed: error code
    // @param error_msg 当 error_code 非 0 时，表示错误描述信息
    // @param tracks track 指针数组
    // @param tracks_count tracks 数组元素个数 
    void (QN_API * on_publish_tracks_result) (int error_code, const char* error_msg, QNRtcTrackInfo* tracks[], size_t tracks_count);
    
    // 订阅远端 tracks 的结果通知
    // @param error_code 0:success, failed: error code
    // @param error_msg 当 error_code 非 0 时，表示错误描述信息
    // @param tracks track 指针数组
    // @param tracks_count tracks 数组元素个数 
    void (QN_API * on_subscribe_tracks_result) (int error_code, const char* error_msg, QNRtcTrackInfo* tracks[], size_t tracks_count);

    // 远端发布 tracks 的通知，收到此通知后，本端便可订阅
    // @param tracks track 指针数组
    // @param tracks_count tracks 数组元素个数 
    void (QN_API * on_remote_add_tracks) (QNRtcTrackInfo* tracks[], size_t tracks_count);

    // 远端取消发布 tracks 的通知，收到此通知后，本端需取消订阅并释放相应的资源
    // @param tracks track id 指针数组
    // @param tracks_count tracks 数组元素个数 
    void (QN_API * on_remote_delete_tracks) (char* tracks[], size_t tracks_count);

    // 远端用户加入房间的通知
    // @param user_id 用户 ID
    // @param user_data 用户自定义数据，服务器仅透传，由 join_room 时传入 
    void (QN_API * on_remote_user_join) (const char* user_id, const char* user_data);

    // 远端用户离开房间的通知
    // @param user_id 用户 ID
    // @param error_code 如果非 0，则表示异常退出,如 10006、10033
    void (QN_API * on_remote_user_leave) (const char* user_id, int error_code);

    // 踢出其它用户的操作结果通知
    // @param kicked_out_user_id 被踢出房间的用户 ID
    // @param error_code 0：操作成功
    // @param error_msg 如果 error_code 非 0，则表示错误描述信息 
    void (QN_API * on_kickout_result) (const char* kicked_out_user_id, int error_code, const char* error_msg);

    // 音频回调接口

    // 音频 PCM 数据回调接口，本地和远端的均会通过此接口进行通知
    // @param audio_data PCM 数据指针
    // @param bits_per_sample 位宽，即每个采样点所占位数
    // @param sample_rate 采样率
    // @param number_of_channels 声道数
    // @param number_of_frames audio_data 中所含采样点数
    // @param user_id 此次回调所属 user ID
    void (QN_API * on_audio_frame) (
        const unsigned char* audio_data, 
        int bits_per_sample,
        int sample_rate,
        int number_of_channels,
        int number_of_frames,
        const char* user_id
        );

    // 音频设备插拔事件通知
    // @param device_state 最新音频设备状态
    // @param 设备 GUID
    void (QN_API * on_audio_device_state_changed)(QNRtcAudioDeviceState device_state, const char* device_guid);
    
    // 视频回调接口

    // 视频帧数据回调
    // 当本地开启摄像头或者订阅了远端视频 Track 后，每一帧视频数据均会触发此回调
    // @param raw_data 数据内存指针
    // @param data_len 数据长度
    // @param width 图像宽度
    // @param height 图像高度
    // @param video_type 图像格式
    // @param track_id 所属 Track Id，自己或者远端用户发布的 Track
    // @param user_id 所属 User'id，自己或者远端用户，优先判断 user id
    void (QN_API * on_video_frame)(
        const unsigned char* raw_data,
        int data_len,
        int width,
        int height,
        QNRtcVideoCaptureType video_type,
        const char* track_id,
        const char* user_id
    );

    // 视频帧数据回调，只有调用本地预览接口时触发
    // @param raw_data 数据内存指针
    // @param data_len 数据长度
    // @param width 图像宽度
    // @param height 图像高度
    // @param video_type 图像格式 
    void (QN_API * on_preview_video_frame)(
        const unsigned char* raw_data,
        int data_len,
        int width,
        int height,
        QNRtcVideoCaptureType video_type
    );

    // 设备插拔事件通知
    // @param device_state 设备状态，插入 or 拔出
    // @param device_name 设备名称 
    void (QN_API * on_video_device_state_changed)(
        QNRtcVideoDeviceState device_state,
        const char* device_name
    );

    // 外置视频编码器时，SDK 内部根据 REMB 反馈实时调整编码器码率和帧率输出
    // @param bitrate 新的编码器输出码率, unit: bps
    // @param frame_rate 新的编码器输出帧率 
    void (QN_API * on_encoder_adjust_setting)(
        int bitrate,
        int frame_rate
    );

    // 视频解码回调，当 SDK 接收到完整的视频帧时，将回调通知上层进行解码
    // @param encoded_frame SDK 将接收到的完整视频帧传递给上层进行解码
    // @param decoded_frame 上层解码后，如果需要渲染可将解码后的数据通过此参数传递会 SDK，否则可忽略 
    void (QN_API * decode_video_frame)(
        const QNRtcEncodedFrame* encoded_frame,
        QNRtcDecodedFrame** decoded_frame
    );
} QNRtcEvent, *PQNRtcEvent;

#ifdef __cplusplus
}
#endif

#endif // __QN_EVENT_H__