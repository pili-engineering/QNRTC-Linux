#ifndef QN_AUDIO_MUSIC_MIXER_H
#define QN_AUDIO_MUSIC_MIXER_H

#include "qn_common_def.h"

namespace qiniu {

// 背景音乐混音状态
enum QNMusicMixerState {
  kStateIdle,       // 初始
  kStateMixing,    // 开始混音
  kStatePaused,     // 暂停
  kStateStopped,    // 停止
  kStateCompleted,  // 完成
};

class QINIU_EXPORT_DLL QNAudioMusicMixerListener {
 public:
  /**
   * 错误回调
   * 
   * @param error_code 错误码
   * @param error_message 错误信息
   */
  virtual void OnAudioMusicMixerError(int error_code,
                                      const std::string& error_message) = 0;
  /**
   * 背景音乐混音状态变化回调
   *
   * @param music_state_ 背景音乐混音状态
   */
  virtual void OnAudioMusicMixerStateChanged(QNMusicMixerState music_state) = 0;

  /**
   * 背景音乐播放进度回调
   *
   * @param current_pos 背景音乐播放当前进度
   */
  virtual void OnAudioMusicMixing(int64_t current_pos) = 0;

 protected:
  ~QNAudioMusicMixerListener(){};
};

class QINIU_EXPORT_DLL QNAudioMusicMixer {
 public:

  /**
   * 获取背景音乐总时长，单位 ms
   */
  static int64_t GetDuration(const std::string& file_path);

  /**
   * 设置背景音乐混音音量，范围 0～1.0
   *
   * @param mix_volume 参与混音的音量
   */
  virtual void SetMixingVolume(float mix_volume) = 0;
    
  /**
   * 获取背景音乐混音音量
   */
  virtual float GetMixingVolume() = 0;

  /**
   * 设置起始位置
   *
   * @param start_pos 起始位置
   */
  virtual void SetStartPosition(int64_t start_pos) = 0;
    
  /**
   * 获取起始位置
   */
  virtual int64_t GetStartPosition() = 0;

  /**
   * 获取当前播放进度，单位 ms
   */
  virtual int64_t GetCurrentPosition() = 0;

  /**
   * 开始混音，默认混音一次
   */
  virtual bool Start() = 0;
    
  /**
   * 结束混音
   */
  virtual bool Stop() = 0;
    
  /**
   * 暂停混音
   */
  virtual bool Pause() = 0;
    
  /**
   * 恢复混音
   */
  virtual bool Resume() = 0;

  /**
   * 配置循环次数并开始混音
   *
   * @param loop_count 循环次数，-1 一直混音，0 不混音，> 0 混音次数
   */
  virtual bool Start(int loop_count) = 0;
    
  /**
   * 跳转播放
   *
   * @param seek_pos 跳转位置
   */
  virtual bool SeekTo(int64_t seek_pos) = 0;

 protected:
  QNAudioMusicMixer(){};
  ~QNAudioMusicMixer(){};
};

}  // namespace qiniu

#endif
