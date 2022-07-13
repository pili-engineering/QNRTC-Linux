#ifndef QN_TRACK_INTERFACE_H
#define QN_TRACK_INTERFACE_H

#include "qn_common_def.h"
#include "qn_audio_music_mixer.h"
#include "qn_audio_effect_mixer.h"

namespace qiniu {
class QNLocalTrack;
class QNRemoteTrack;
class QNRemoteVideoTrack;
class QNRemoteAudioTrack;
class QNRemoteUser;

typedef std::list<QNLocalTrack*> LocalTrackList;
typedef std::list<QNRemoteTrack*> RemoteTrackList;
typedef std::list<QNRemoteVideoTrack*> RemoteVideoTrackList;
typedef std::list<QNRemoteAudioTrack*> RemoteAudioTrackList;
typedef std::list<QNRemoteUser> RemoteUserList;

class QINIU_EXPORT_DLL QNCameraEventListener {
 public:
  /**
   * 相机失败回调
   *
   * @param error_code 错误码
   * @param error_message 失败描述
   */
  virtual void OnCameraError(int error_code,
                             const std::string& error_message){};
};

class QNRemoteUser {
 public:
  /**
   * 远端用户的 userId
   */
  std::string user_id;

  /**
   * 远端用户数据
   */
  std::string user_data;

  /**
   * 远端用户发布的音频 track 列表
   */
  RemoteAudioTrackList remote_audio_track_list;

  /**
   * 远端发布的视频 track 列表
   */
  RemoteVideoTrackList remote_video_track_list;
};

class QINIU_EXPORT_DLL QNAudioFrameListener {
 public:
  /**
   * 音频数据回调
   *
   * @param user_id 此音频数据所属的用户
   * @param track_id 此音频 track id
   * @param data 音频数据内存指针
   * @param data_size 数据长度
   * @param bits_per_sample 位宽，即每个采样点占用位数
   * @param sample_rate 采样率
   * @param channels 声道数
   */
  virtual void OnAudioFrame(const string& user_id, const string& track_id,
                            const uint8_t* data, uint32_t data_size,
                            uint32_t bits_per_sample, uint32_t sample_rate,
                            uint32_t channels) = 0;

 protected:
  virtual ~QNAudioFrameListener() {}
};

class QINIU_EXPORT_DLL QNVideoFrameListener {
 public:
  /**
   * 视频数据回调
   *
   * @param user_id 此视频频数据所属的用户
   * @param track_id 视频 track id
   * @param data 视频数据内存指针
   * @param data_size 数据长度
   * @param width 视频宽
   * @param height 视频高
   * @param video_frame_type 视频数据类型
   */
  virtual void OnVideoFrame(const std::string& user_id, const string& track_id,
                            const uint8_t* data, uint32_t data_size,
                            int32_t width, int32_t height,
                            QNVideoFrameType video_frame_type) = 0;

 protected:
  ~QNVideoFrameListener() {}
};

class QINIU_EXPORT_DLL QNTrackInfoChangedListener {
 public:
  /**
   * 视频 Track profile 改变后触发
   *
   * @param track_id 当前的 track_id
   * @param profile 当前的 profile
   */
  virtual void OnVideoProfileChanged(const std::string& user_id,
                                     const std::string& track_id,
                                     QNTrackProfile profile) = 0;

  /**
   * Track 静默状态改变后触发
   *
   * @param track_id 当前的 track_id
   * @param muted Track 当前的静默状态
   */
  virtual void OnMuteStateChanged(const std::string& user_id,
                                  const std::string& track_id, bool muted) = 0;

 protected:
  ~QNTrackInfoChangedListener() {}
};

class QINIU_EXPORT_DLL QNTrack {
 public:
  /**
   * 获取 TrackId
   */
  virtual const std::string& GetTrackID() = 0;

  /**
   * 获取 userId
   */
  virtual const std::string& GetUserID() = 0;

  /**
   * 获取 Tag
   */
  virtual const std::string& GetTag() = 0;

  /**
   * 本 track 是否为音频 track
   */
  virtual bool IsAudio() = 0;

  /**
   * 本 track 是否为视频 track
   */
  virtual bool IsVideo() = 0;

  /**
   * 是否 muted
   */
  virtual bool IsMuted() = 0;

 protected:
  virtual ~QNTrack() {}
};

class QINIU_EXPORT_DLL QNLocalTrack : public virtual QNTrack {
 public:
  /**
   * 设置静默状态
   */
  virtual void SetMuted(bool muted) = 0;

 protected:
  virtual ~QNLocalTrack() {}
};

class QINIU_EXPORT_DLL QNLocalAudioTrack : public virtual QNLocalTrack {
 public:
  /**
   * 设置采集的音频数据监控回调接口
   *
   * @param listener QNAudioFrameListener 派生类实例指针
   */
  virtual void SetAudioFrameListener(QNAudioFrameListener* listener) = 0;

  /**
   * 设置采集音频音量（不改变系统设备的音量）
   *
   * @param 音量大小，[0.0 - 10.0], 默认为 1.0
   */
  virtual void SetVolume(double volume) = 0;

  /**
   * 获取音频采集的音量
   *
   * @return 音量值
   */
  virtual float GetVolumeLevel() = 0;

 protected:
  ~QNLocalAudioTrack() {}
};

class QINIU_EXPORT_DLL QNMicrophoneAudioTrack
    : public virtual QNLocalAudioTrack {
 public:

  /**
   * 获取背景音乐混音对象实例
   *
   * @param music_path 背景音乐文件路径
   * @param listener 背景音乐混音回调监听
   * @return QNAudioMusicMixer 实例对象
   */
  virtual QNAudioMusicMixer* CreateAudioMusicMixer(
      const std::string& music_path, QNAudioMusicMixerListener* listener) = 0;

  /**
   * 释放背景音乐混音对象实例
   *
   * @param mixer QNAudioMusicMixer 实例对象
   */
  virtual void DestroyAudioMusicMixer(QNAudioMusicMixer* mixer) = 0;

  /**
   * 获取音效混音对象实例
   *
   * @param listener 音效混音回调监听
   * @return QNAudioEffectMixer 实例对象
   */
  virtual QNAudioEffectMixer* CreateAudioEffectMixer(
      QNAudioEffectMixerListener* listener) = 0;

  /**
   * 释放音效混音对象实例
   *
   * @param mixer QNAudioEffectMixer 实例对象
   */
  virtual void DestroyAudioEffectMixer(QNAudioEffectMixer* mixer) = 0;
 protected:
  ~QNMicrophoneAudioTrack() {}
};

class QINIU_EXPORT_DLL QNCustomAudioTrack : public virtual QNLocalAudioTrack {
 public:
  /**
   * 推送自定义音频帧
   *
   * @param data 音频数据
   * @param data_size 数据长度
   * @param bits_per_sample 位宽，即每个采样点占用位数
   * @param sample_rate 采样率
   * @param channels 声道数
   */
  virtual int32_t PushAudioFrame(const uint8_t* data, uint32_t data_size,
                                 uint32_t bits_per_sample, uint32_t sample_rate,
                                 uint32_t channels) = 0;

 protected:
  ~QNCustomAudioTrack() {}
};

class QINIU_EXPORT_DLL QNLocalVideoTrack : public virtual QNLocalTrack {
 public:
  /**
   * 设置采集的视频数据监控回调接口
   *
   * @param listener QNLocalVideoFrameListener 派生类实例指针
   */
  virtual void SetVideoFrameListener(QNVideoFrameListener* listener) = 0;

  /**
   * 设置自定义 video sei 数据插入，在使用合流功能时插入SEI，必须要保证
   * 合流时设置的帧率不超过连麦时的帧率。
   *
   * @param message 被插入的 sei 数据
   * @param uuid 16 个字节, 如果传空字符串或者字符串长度不为 16
   * 个字节，则使用默认的 uuid
   * @param repeat_count  当前 sei 数据被插入的次数，-1 表示持续插入
   */
  virtual void SendSEI(std::string& message, std::string& uuid,
                       int32_t repeat_count) = 0;

  /**
   * 开启原始帧裁剪功能，设置参数要求如下，如果设置不正确，则输出原始图像
   *
   * @param crop_x 开始裁减的 X 坐标点，原点为左上角，必须落在原图之内
   * @param crop_y 开始裁减的 Y 坐标点，原点为左上角，必须落在原图之内
   * @param crop_width 目标图像宽度，必须为 4 的整数倍，如果是裁剪模式，crop_x
   * 和 crop_y，裁剪图像必须在原始图像之内
   * @param crop_height 目标图像高度，必须为 4 的整数倍，如果是裁剪模式，crop_x
   * 和 crop_y，裁剪图像必须在原始图像之内
   */
  virtual void StartCrop(uint32_t crop_x, uint32_t crop_y, int32_t crop_width,
                         int32_t crop_height) = 0;

  /**
   * 关闭裁剪功能
   */
  virtual void StopCrop() = 0;

  /**
   * 开启原始帧缩放功能，设置参数要求如下，如果设置不正确，则输出原始图像
   *
   * @param dst_width 目标图像宽度，必须为 4 的整数倍
   * @param dst_height 目标图像高度，必须为 4 的整数倍
   */
  virtual void StartScale(int32_t dst_width, int32_t dst_height) = 0;

  /**
   * 关闭缩放功能
   */
  virtual void StopScale() = 0;

 protected:
  virtual ~QNLocalVideoTrack() {}
};

class QINIU_EXPORT_DLL QNCameraVideoTrack : public virtual QNLocalVideoTrack {
 public:
  /**
   * 设置摄像头事件回调
   */
  virtual void SetCameraEventListener(QNCameraEventListener* listener) = 0;

  /**
   * 开启摄像头采集
   */
  virtual void StartCapture() = 0;

  /**
   * 关闭摄像头采集
   */
  virtual void StopCapture() = 0;

  /**
   * 设置本地采集画面是否镜像，开启镜像后，订阅端看到的是镜像画面
   *
   * @param mirror 是否镜像，true or false
   */
  virtual void SetMirror(bool mirror) = 0;

  /**
   * 将摄像头采集流替换为图片流，此接口需要在摄像头推流成功后调用。
   *
   * @param object 推送图片的对象，可以是文件对象、文件路径、url，Window
   * 平台只支持文件路径, 设置为 nullptr
   * 时，取消推送图片，关闭图片推送，恢复摄像头采集画面
   * @param type 图片格式，Windows 平台只支持 kImagJpeg
   */
  virtual void PushImage(void* object, QNImageType type) = 0;

 protected:
  virtual ~QNCameraVideoTrack() {}
};

class QINIU_EXPORT_DLL QNCustomVideoTrack : public virtual QNLocalVideoTrack {
 public:
  /**
   * 推送自定义视频帧
   *
   * @param data 视频数据
   * @param data_size 数据长度
   * @param width 图像宽度
   * @param height 图像高度
   * @param timestamp_us 时间戳，注意单位为:微妙
   * @param video_frame_type 视频原始格式，目前支持：kI420 kYUY2 kRGB24
   * @param rotation 导入后旋转角度，如果不需要旋转则使用默认值 kVideoRotation0
   * 即可
   * @param mirror 导入后是否镜像
   */
  virtual int32_t PushVideoFrame(const uint8_t* data, const uint32_t data_size,
                                 const int32_t width, const int32_t height,
                                 const int64_t timestamp_us,
                                 QNVideoFrameType video_frame_type,
                                 QNVideoRotation rotation,
                                 bool mirror = false) = 0;

 protected:
  virtual ~QNCustomVideoTrack() {}
};

class QINIU_EXPORT_DLL QNRemoteTrack : public virtual QNTrack {
 public:
  /**
   * 设置远端 track 状态监控回调接口
   *
   * @param listener QNTrackInfoChangedListener 派生类实例指针
   */
  virtual void SetTrackInfoChangedListener(
      QNTrackInfoChangedListener* listener) = 0;

  /**
   * 是否已订阅
   */
  virtual bool IsSubscribed() = 0;

 protected:
  virtual ~QNRemoteTrack() {}
};

class QINIU_EXPORT_DLL QNRemoteAudioTrack : public virtual QNRemoteTrack {
 public:
  /**
   * 设置远端音频数据监控回调接口
   *
   * @param listener QNRemoteAudioFrameListener 派生类实例指针
   */
  virtual void SetAudioFrameListener(QNAudioFrameListener* listener) = 0;

  /**
   * 设置远端音频音量（不改变系统设备的音量）
   *
   * @param 音量大小，[1.0-10.0], 默认为 1.0
   */
  virtual void SetVolume(double volume) = 0;

  /**
   * 获取音频播放的音量
   *
   * @return 音量值
   */
  virtual float GetVolumeLevel() = 0;

 protected:
  ~QNRemoteAudioTrack() {}
};

class QINIU_EXPORT_DLL QNRemoteVideoTrack : public virtual QNRemoteTrack {
 public:
  /**
   * 设置远端视频数据监控回调接口
   *
   * @param listener QNRemoteVideoFrameListener 派生类实例指针
   */
  virtual void SetVideoFrameListener(QNVideoFrameListener* listener) = 0;

  /**
   * 远端视频是否开启多流
   */
  virtual bool IsMultiProfileEnabled() = 0;

  /**
   * 多流开启的情况下，可以设置订阅不同 profile 的流
   *
   * @param profile 需要设置的 profile
   */
  virtual void SetProfile(QNTrackProfile profile) = 0;

 protected:
  ~QNRemoteVideoTrack() {}
};

}  // namespace qiniu

#endif  // QN_TRACK_INTERFACE_H
