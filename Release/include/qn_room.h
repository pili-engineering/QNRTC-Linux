#ifndef __QN_ROOM_H__
#define __QN_ROOM_H__
#include "qn_event.h"

#ifdef __cplusplus
extern "C" {
#endif
    // 初始化配置日志输出
    // @param event_ptr 用于接收 SDK 异步通知的 QNRtcEvent 结构体指针，非线程安全，离开房间前不可修改
    // @param log_level 记录日志级别
    // @param log_file_name 日志文件名，如果为空，则不输出日志文件
    // @return 0:成功，其它请参考错误码列表
    QINIU_EXPORT_DLL_C int qn_rtc_init(QNRtcEvent* event_ptr, QNRtcQNLogLevel log_level, char* log_file_name);

    // 释放 SDK 所有资源，与 qn_rtc_init 成对调用
    QINIU_EXPORT_DLL_C void qn_rtc_uninit();

    // 配置信令信令间隔，影响监控网络断开的灵敏度，需在 qn_rtc_join_room 前调用
    // 间隔越低，越可以更快的检测到网络断开，默认为 3
    // @param interval_seconds 信令心跳间隔，可设置范围为 1~ 10；单位：秒 
    QINIU_EXPORT_DLL_C void qn_rtc_set_heartbeat_interval(int interval_seconds);

    // 加入房间
    // @param room_token RoomToken，由上层向 APPServer 获取
    // @param user_data 用户自定义数据，服务端会向其它端透传
    // @param cb 加入房间结果异步通知，只有当返回 0 时才会触发
    // @return 仅当返回 0 时才会触发 OnJoinResult 回调，否则请查看错误码列表 
    QINIU_EXPORT_DLL_C int qn_rtc_join_room(char* room_token, char* user_data);

    // 离开房间，此为同步方法，没有异步回调
    // @return 0:成功；其它请参考错误码列表 
    QINIU_EXPORT_DLL_C int qn_rtc_leave_room();

    // 是否已加入了房间
    // @return 1:已加入； 0：未加入
    QINIU_EXPORT_DLL_C int qn_rtc_is_join();

    // 发布音、视频 Track，此方法为异步方法，仅当返回 0 时才会触发 on_publish_tracks_result 回调
    // @param tracks 音视频 Track 指针数组
    // @param tracks_count tracks 指针数组中元素个数
    // @return 0:成功；其它请参考错误码列表 
    QINIU_EXPORT_DLL_C int qn_rtc_publish(QNRtcTrackInfo* tracks[], size_t tracks_count);

    // 取消发布音、视频 Track，此方法为同步方法
    // @param tracks_id 音视频 Track id 指针数组
    // @param tracks_count tracks_id 指针数组中元素个数
    // @return 0:成功；其它请参考错误码列表 
    QINIU_EXPORT_DLL_C int qn_rtc_unpublish(char* tracks_id[], size_t tracks_id_count);

    // 目前仅针对嵌入式设备中使用外置编码器时对 SDK 内部参数的初始化
    // 目前使用外置编码器非线程安全，仅可同时编码一路视频 Track
    // 调用时机：调用 qn_rtc_publish 并收到 on_publish_tracks_result 成功回调后
    // @param width 图像宽度
    // @param height 图像高度
    // @param max_fps 最大帧率
    // @param max_bitrate 最大码率
    // @param adjust_func 配置上层编码器回调
    // @return 0：成功； 其它请参考错误码列表 
    QINIU_EXPORT_DLL_C int qn_rtc_init_encoder_params(
        int width, 
        int height, 
        int max_fps, 
        int max_bitrate
    );

    // 目前仅针对嵌入式设备中使用外置编码器时将外部 H.264 Nalu 导入 SDK 进行视频交互
    // @param nalus_ptr QNRtcH264Nal 指针数组
    // @param nalus_size nalus_ptr 中数组元素个数
    // @param is_key_frame 会否是关键帧，1：是； 0：否；sps 和 pps 也要置为 1
    // @return 0：成功； 其它请参考错误码列表 
    QINIU_EXPORT_DLL_C int qn_rtc_input_h264_frame(QNRtcH264Nal** nalus_ptr, size_t nalus_size, unsigned char is_key_frame);

    // 订阅远端已发布的音、视频 Track，此方法为异步方法，仅当返回 0 时才会触发 on_subscribe_tracks_result 回调
    // @param tracks 音视频 Track 指针数组
    // @param tracks_count tracks 指针数组中元素个数
    // @return 0:成功；其它请参考错误码列表 
    QINIU_EXPORT_DLL_C int qn_rtc_subscribe(QNRtcTrackInfo* tracks[], size_t tracks_count);

    // 取消订阅音、视频 Track，此方法为同步方法
    // @param tracks_id 音视频 Track id 指针数组
    // @param tracks_count tracks_id 指针数组中元素个数
    // @return 0:成功；其它请参考错误码列表 
    QINIU_EXPORT_DLL_C int qn_rtc_unsubscribe(char* tracks_id[], size_t tracks_id_count);

    // 静默本地已发布音频 Track，静默后，远端将产生静音效果
    // @param mute_flag 是否静默本地音频 track
    // @return 0:成功；其它请参考错误码列表 
    QINIU_EXPORT_DLL_C int qn_rtc_mute_audio(int mute_flag);

    // 静默本地已发布视频 Track，静默后，远端看到的为黑屏
    // @param track_id 本地视频 track id
    // @param mute_flag 是否静默本地视频 track
    // @return 0:成功；其它请参考错误码列表 
    QINIU_EXPORT_DLL_C int qn_rtc_mute_video(char* track_id, int mute_flag);

    // 配置此房间内指定音视频 Track 在服务端合流转推（RTMP 流）时的位置布局
    // 合流转推的画布大小在 qiniu.com ==》“管理控制台” ==》“实时音视频云” 的指定连麦应用中配置
    // @param added_tracks_opt 新增 Track 及其在合流画面中的位置和大小，更新位置、大小时也用此参数
    // @param added_tracks_count added_tracks_opt 指针数组中的元素个数
    // @param remove_tracks_id 取消 Track 在合流画面的合流
    // @param remove_tracks_id_count remove_tracks_id 指针数组中的元素个数
    // @return 0:成功；其它请参考错误码列表 
    QINIU_EXPORT_DLL_C int qn_rtc_set_merge_stream_layout(
        QNRtcMergeOptInfo* added_tracks_opt[], 
        size_t added_tracks_count, 
        char* remove_tracks_id[], 
        size_t remove_tracks_id_count
    );

    // 停止所有此房间内的所有合流任务
    // 房间未连接的情况下停止无效
    // @return 0:操作成功，具体合流结果请通过观看旁路直播进行查看 
    QINIU_EXPORT_DLL_C int qn_rtc_stop_merge_stream();

    // 枚举音频设备信息
    // @param device_type: 0:record; 1: playout
    // @cb 用于接收枚举结果的回调接口函数指针
    // @return success:>= 0; 失败请参考错误码列表
    QINIU_EXPORT_DLL_C int qn_rtc_enum_audio_devices(
        int device_type, 
        on_audio_device_enum cb
    );

    // 枚举视频设备信息
    // @cb 用于接收枚举结果的回调接口函数指针
    // @return success:>= 0; 失败请参考错误码列表
    QINIU_EXPORT_DLL_C int qn_rtc_enum_video_devices(on_camera_device_enum cb);

    // 踢出房间内指定用户，需要有踢人权限，在 APP 层获取 RoomToken 时指定
    // @param user_id 用于被踢出的用户 Id
    // @return 成功返回 0，失败请参考错误码列表
    // @brief 当返回 0 时，最终操作结果由 on_kickout_result 回调通知
    QINIU_EXPORT_DLL_C int qn_rtc_kickout_user(char* user_id);

    // 配置媒体传输通道底层传输协议，默认为 forceUdp，当用户网络下 UDP 不通时，可配置使用 forceTcp 或 preferUdp
    // @param policy_ value of Enum:IcePolicy
    QINIU_EXPORT_DLL_C void qn_rtc_set_ice_policy(QNRtcIcePolicy policy);

    // 是否开启多波段分频，默认开启，禁用后可去除多人同时说话时的互相抑制问题
    // @param enable_ 是否开启， 1 or 0
    // @return 目前阶段全部返回 0
    // @brief 测试阶段，慎用
    QINIU_EXPORT_DLL_C int qn_rtc_enable_multiband(int enable);

    // 是否开启回声消除，默认开启，禁用后在使用扬声器的场景下将产生回声
    // @param enable_ 是否开启， 1 or 0
    // @return 目前阶段全部返回 0
    // @brief 测试阶段，慎用
    QINIU_EXPORT_DLL_C int qn_rtc_enable_aec(int enable);

#ifdef __cplusplus
}
#endif

#endif // __QN_ROOM_H__