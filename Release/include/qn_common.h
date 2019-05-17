#ifndef __QN_COMMON_H__
#define __QN_COMMON_H__
#include <stdio.h>
#include "qn_error_code.h"

#ifdef __cplusplus
extern "C" {
#endif
#if defined _WIN32
#ifdef __QINIU_DLL_EXPORT_
#define QN_API __cdecl
#define QN_CBI __stdcall
#define QINIU_EXPORT_DLL_C __declspec(dllexport)
#else 
#define QN_API
#define QN_CBI
#define QINIU_EXPORT_DLL_C 
#endif // __QINIU_DLL_EXPORT_
#else
#ifdef __QINIU_DLL_EXPORT_
#define QN_API
#define QN_CBI
#define QINIU_EXPORT_DLL_C __attribute__((visibility("default")))
#else 
#define QN_API
#define QN_CBI
#define QINIU_EXPORT_DLL_C
#endif // __QINIU_DLL_EXPORT_
#endif // WIN32

#define KIND_AUDIO "audio"
#define KIND_VIDEO "video"

    // 音频设备配置，用于指定设备进行录制或者播放 
    typedef struct
    {
        enum WindowsDeviceType
        {
            wdt_DefaultCommunicationDevice = -1, // 通信设备 
            wdt_DefaultDevice              = -2  // 普通设备 
        }device_type;
        int device_index;                        // 设备编号；注意：0 不代表默认设备 
    }QNRtcAudioDeviceSetting;

    // 音频设备当前状态，用与设备插拔的检测和通知 
    typedef enum
    {
        ads_active     = 0x00000001,     // 新的可用设备 
        ads_disabled   = 0x00000002,     // 设备失效 
        ads_notpresent = 0x00000004,     // 设备不存在 
        ads_unplugged  = 0x00000008,     // 设备被拔出 
        ads_MASK_ALL   = 0x0000000F,
    }QNRtcAudioDeviceState;

    // 音频设备信息 
    typedef struct
    {
        enum AudioDeviceType
        {
            adt_invalid = -1,
            adt_record,
            adt_playout,
        }device_type;
        int  device_index;
        int  is_default;
        char device_name[128];
        char device_id[128];
    }QNRtcAudioDeviceInfo;

    // 视频数据格式 
    typedef enum
    {
        kUnknown,
        kI420,
        kIYUV,
        kRGB24,
        kABGR,
        kARGB,
        kARGB4444,
        kRGB565,
        kARGB1555,
        kYUY2,
        kYV12,
        kUYVY,
        kMJPEG,
        kNV21,
        kNV12,
        kBGRA,
    }QNRtcVideoCaptureType;

    // 摄像头支持的采集能力 
    typedef struct
    {
        int                     width;
        int                     height;
        int                     max_fps;
        QNRtcVideoCaptureType   video_type;
    }QNRtcCameraCapability;

    // 摄像头设备信息 
    typedef struct
    {
        char*                   device_id;      // 设备 Id，用于系统内做标识 
        char*                   device_name;    // 设备名，用于展示给用户，较友好 
        int                     capability_count;
        QNRtcCameraCapability** capabilitys;    // 此摄像头所支持的采集能力列表 
    }QNRtcCameraDeviceInfo;

    // 摄像头预览配置，在调用 PreviewCamera 接口时使用 
    typedef struct
    {
        char*       device_id;              // 设备 Id
        char*       device_name;            // 设备名 
        int         width;
        int         height;
        int         max_fps;
        void*       render_hwnd;            //video render window hwnd,MFC:HWND; QT:winId
    }QNRtcCameraSetting;

    // 摄像头设备状态，SDK 提供设备插拔状态的监控，以下用于标识设备被插入还是拔出
    // 设备状态变化后，建议通过 GetCameraCount 重新获取摄像头设备列表 
    typedef enum
    {
        vds_active = 0x00000001,            // 设备插入 
        vds_lost   = 0x00000002,            // 设备拔出 
    }QNRtcVideoDeviceState;

    // 表示原始视频数据的旋转角度，主要用于对原始视频数据进行处理的功能接口中 
    typedef enum
    {
        kVideoRotation_0   = 0,
        kVideoRotation_90  = 90,
        kVideoRotation_180 = 180,
        kVideoRotation_270 = 270
    }QNRtcVideoRotation;

    // SDK 提供对桌面和窗口的画面采集，以下为可以采集的屏幕或窗口信息 
    typedef struct
    {
        int     id;        // 窗口 Id，唯一标识 
        char*   title;     // 窗口标题 
        int     is_screen; // 1:显示器; 0:窗口 
    }QNRtcScreenWindowInfo;

    // SDK 日志等级 
    typedef enum
    {
        LOG_DEBUG = 0,
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR,
    }QNRtcQNLogLevel;

    // 房间连接状态 
    typedef enum
    {
        rs_idle,            // 空闲，未连接 
        rs_connecting,      // 连接中 
        rs_connected,       // 已连接 
        rs_reconnecting,    // 已断开，并在自动重连中(SDK 内部自动重连) 
    }QNRtcRoomState;

    // Track 类型，或者可以理解为数据源类型 
    typedef enum
    {
        tst_Invalid = -1,   // 无效类型 
        tst_Microphone,     // 音频数据：数据源为 SDK 内部的麦克风采集 
        tst_ExternalPCM,    // 音频数据：数据源为外部导入的 PCM 数据 
        tst_Camera,         // 视频数据: 数据源为摄像头采集 
        tst_ScreenCasts,    // 视频数据：数据源为屏幕采集或窗口采集 
        tst_ExternalYUV,    // 视频数据：数据源为外部数据导入，YUV 或 RGB 格式 
    }QNRtcTrackSourceType;

    // 旁路直播合流配置信息，通过 SDK 将参数发送到服务端 
    // 服务端按照指定的参数进行合流并推出 RTMP 流 
    typedef struct
    {
        char*  track_id;     // Track Id，房间内唯一 
        int    is_video;     // 是否为视频类型，如果为 0 则以下参数无效 
        int    pos_x;        // 此路流（即此 Track）在 RTMP 流画布中的 X 坐标 
        int    pos_y;        // 此路流（即此 Track）在 RTMP 流画布中的 Y 坐标 
        int    pos_z;        // 此路流（即此 Track）在 RTMP 流画布中的 Z 坐标 
        int    width;        // 此路流（即此 Track）在 RTMP 流画布中的宽度，缩放、裁减方式根据后端配置决定 
        int    height;       // 此路流（即此 Track）在 RTMP 流画布中的高度，缩放、裁减方式根据后端配置决定 
    }QNRtcMergeOptInfo;

    // 用户信息 
    typedef struct
    {
        char*  user_id;      // 用户 Id，房间内唯一标识，同一房间不能重复登录 
        char*  user_data;    // 用户自定义数据，在 JoinRoom 时指定，通过服务器进行透传 
    }QNRtcUserInfo;

    // 通话质量回调信息 
    typedef struct
    {
        char*   track_id; // Track Id，房间内唯一 
        char*   user_id;  // 此 track 所属 user id
        int     is_video; // 是否为视频 track 

        // 以下两个成员为音频 track 参数，仅当 is_video 为 false 时有效 
        int     audio_bitrate;              // 音频码率，单位：bps
        float   audio_packet_lost_rate;     // 音频丢包率 

        // 以下成员为视频 track 参数，仅当 is_video 为 true 时有效 
        int     video_width;                // 视频宽度 
        int     video_height;               // 视频高度 
        int     video_frame_rate;           // 视频帧率 
        int     video_bitrate;              // 码率，单位：bps
        float   video_packet_lost_rate;     // 丢包率 
    }QNRtcStatisticsReport;

    // Track 描述信息 
    typedef struct
    {
        char*  track_id;    // Track Id（数据流唯一标识） 
        char*  local_id;    // Track 在 webrtc 中的标识 
        char*  user_id;     // 此 Track 所属 User Id
        char*  kind;        // VIDEO_KIND_TYPE 或 AUDIO_KIND_TYPE
        char*  tag;         // Track 自定义 Tag，由用户指定并使用，SDK 仅做透传 
        int    is_master;   // 是否为主流，默认为 0
        int    is_muted;    // 是否已静默 
        int    max_bitrate; // 最大码率，单位：bps
        int    state;
        int    is_connected;// 此 track 当前是否处于已连接状态 
        
        // 以下成员仅对视频 Track 有效 
        int    width;
        int    height;
        int    max_fps;
        void*  render_hwnd;      // 渲染窗口句柄，HWND 或 winId(),为空则不渲染，对数据回调无影响 
        char*  camera_device_id; // 摄像头设备 Id，仅当 source_type 为 tst_Camera 时有效 
        QNRtcTrackSourceType source_type;
        unsigned long long start_tp;
    }QNRtcTrackInfo;

    // 当导入 H.264 裸码流时，用于封装单个 Nal 
    // 目前仅针对 Linux SDK 有效 
    typedef struct
    {
        unsigned char* payload;
        size_t payload_size;
    }QNRtcH264Nal;

    // 已编码视频帧，用于在使用外置视频解码器时，回调 SDK 接收到的完整视频帧给上层用于解码
    typedef struct
    {
        unsigned int encoded_width;
        unsigned int encoded_height;
        unsigned int time_stamp;
        long long ntp_time_ms;      // NTP time of the capture time in local timebase in milliseconds.
        long long capture_time_ms;
        char is_key_frame;
        QNRtcVideoRotation rotation;
        unsigned char* buffer;
        size_t buffer_size;        // playload size
    }QNRtcEncodedFrame;

    // 已解码视频帧，用于在使用外置视频解码器时，上层解码后传递给 SDK
    typedef struct 
    {
        unsigned char* buffer;
        size_t buffer_size;
        size_t width;
        size_t height;
        unsigned long long timestamp_us;
        QNRtcVideoRotation rotation;
        QNRtcVideoCaptureType raw_type;
    }QNRtcDecodedFrame;

    // 媒体传输协议配置 
    typedef enum
    {
        forceUdp  = 0,      // media transfer forced use udp
        forceTcp  = 1,      // media transfer forced use tcp
        preferUdp = 2,      // media transfer use udp first, and if udp don't work, then downgrade using tcp
    }QNRtcIcePolicy;

#ifdef __cplusplus
}
#endif

#endif // __QN_COMMON_H__