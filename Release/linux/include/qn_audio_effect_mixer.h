#ifndef QN_AUDIO_EFFECT_MIXER_H
#define QN_AUDIO_EFFECT_MIXER_H

#include "qn_common_def.h"

namespace qiniu {
class QINIU_EXPORT_DLL QNAudioEffect {
 public:

  /**
   * 通过文件路径，获取音效总时长，单位 ms
   *
   * @param file_path 音效文件地址
   */
  static int64_t GetDuration(const std::string& file_path);

  /**
   * 获取音效标识
   */
  virtual int GetID() = 0;
    
  /**
   * 获取文件路径
   */
  virtual const std::string& GetFilePath() = 0;

  /**
   * 设置播放起始位置
   *
   * @param start_pos 起始位置
   */
  virtual void SetStartPosition(int64_t start_pos) = 0;
    
  /**
   * 获取播放起始位置
   */
  virtual int64_t GetStartPosition() = 0;

  /**
   * 设置播放循环次数
   *
   * @param loop_count 循环次数，-1 一直混音，0 不混音，> 0 混音次数
   */
  virtual void SetLoopCount(int loop_count) = 0;
    
  /**
   * 获取播放循环次数
   */
  virtual int GetLoopCount() = 0;

 protected:
  ~QNAudioEffect(){};
};

class QINIU_EXPORT_DLL QNAudioEffectMixerListener {
 public:
  /**
   * 错误回调
   *
   * @param error_code    错误码
   * @param error_message 错误信息
   */
  virtual void OnAudioEffectMixerError(int error_code,
                                       const std::string& error_message) = 0;
    
  /**
   * 某音效播放完成的回调
   *
   * @param effect_id 音效 id
   */
  virtual void OnAudioEffectFinished(int effect_id) = 0;
 protected:
  ~QNAudioEffectMixerListener(){};
};

class QNAudioEffectMixer {
 public:

  /**
   * 创建 QNAudioEffect 实例对象
   *
   * @param effect_id 音效 id，必须保证唯一性
   * @param file_path 音效文件地址
   */
  virtual QNAudioEffect* CreateAudioEffect(int effect_id, const std::string& file_path) = 0;

  /**
   * 释放 QNAudioEffect 实例对象
   */
  virtual void DestroyAudioEffect(QNAudioEffect* ptr) = 0;

  /**
   * 开始播放
   *
   * @param effect_id 音效 id
   */
  virtual bool Start(int effect_id) = 0;
    
  /**
   * 停止播放
   *
   * @param effect_id 音效 id
   */
  virtual bool Stop(int effect_id) = 0;
    
  /**
   * 暂停播放
   *
   * @param effect_id 音效 id
   */
  virtual bool Pause(int effect_id) = 0;
    
  /**
   * 恢复播放
   *
   * @param effect_id 音效 id
   */
  virtual bool Resume(int effect_id) = 0;

  /**
   * 获取某音效当前播放进度
   *
   * @param effect_id 音效 id
   */
  virtual int64_t GetCurrentPosition(int effect_id) = 0;

  /**
   * 设置某音效音量
   *
   * @param effect_id 音效 id
   * @param volume 音效音量，范围 0～1.0
   */
  virtual void SetVolume(int effect_id, float volume) = 0;
    
  /**
   * 获取音量，未设置时，默认为 1.0
   *
   * @param effect_id 音效 id
   */
  virtual float GetVolume(int effect_id) = 0;

  /**
   * 设置所有音效音量
   *
   * @param volume 音量
   */
  virtual void SetAllEffectsVolume(float volume) = 0;

  /**
   * 全部停止
   */
  virtual bool StopAll() = 0;

  /**
   * 全部暂停
   */
  virtual bool PauseAll() = 0;
    
  /**
   * 全部恢复
   */
  virtual bool ResumeAll() = 0;

 protected:
  QNAudioEffectMixer(){};
  ~QNAudioEffectMixer(){};
};

}  // namespace qiniu

#endif
