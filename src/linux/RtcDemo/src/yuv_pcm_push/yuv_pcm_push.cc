#include "../config.h"
#include "simple_client.h"
#include <cassert>
#include <endian.h>
#include <iostream>
#include <thread>

using namespace std;
using namespace qiniu;

class YuvPcmPush : public SimpleClient, public qiniu::QNPublishResultCallback {

public:
  explicit YuvPcmPush(Config &config) : config_(config) {}

  ~YuvPcmPush() = default;

  void Join(string token, string user_data) override {
    if (joined_) {
      return;
    }
    client_->Join(token, user_data);
  }

  void Leave() override {
    if (joined_) {
      if (published_) {
        LocalTrackList track_list;
        track_list.push_front(audio_track_);
        track_list.push_front(video_track_);
        client_->UnPublish(track_list);
        published_ = false;
        if (video_thread_) {
          video_thread_->join();
        }
        if (audio_thread_) {
          audio_thread_->join();
        }
      }
      client_->Leave();
      this_thread::sleep_for(chrono::milliseconds(1000));
      joined_ = false;
    }
    // 销毁创建的音视频 Track
    if (audio_track_) {
      QNRTC::DestroyLocalTrack(audio_track_);
    }
    if (video_track_) {
      QNRTC::DestroyLocalTrack(video_track_);
    }
  }

protected:
  void OnConnectionStateChanged(
      qiniu::QNConnectionState state,
      const qiniu::QNConnectionDisconnectedInfo *info) override {
    cout << "------------------OnConnectionStateChanged state:" << state
         << endl;
    // 连接状态变化
    if (state == QNConnectionState::kConnected ||
        state == QNConnectionState::kReconnected) {
      joined_ = true;
      if (published_) {
        return;
      }
      LocalTrackList local_track_list;

      // custom audio track
      QNCustomAudioTrackConfig custom_audio_track_config;
      custom_audio_track_config.audio_quality = {
          config_.audio.sample_rate, config_.audio.channels,
          config_.audio.bits_per_sample, config_.audio.sample_rate};
      audio_track_ = QNRTC::CreateCustomAudioTrack(custom_audio_track_config);
      local_track_list.push_front(audio_track_);

      // custom video track
      QNCustomVideoTrackConfig custom_video_track_config;
      custom_video_track_config.encoder_config = {
          config_.yuv_video.width, config_.yuv_video.height,
          config_.yuv_video.fps,
          config_.yuv_video.width * config_.yuv_video.height * 2};
      video_track_ = QNRTC::CreateCustomVideoTrack(custom_video_track_config);
      local_track_list.push_front(video_track_);

      client_->Publish(local_track_list, this);
    } else {
      joined_ = false;
    }
  }

  void OnPublished() override {
    cout << "------------------OnPublished" << endl;
    published_ = true;
    // start push custom a/v
    StartCustomVideoPush();
    StartCustomAudioPush();
  }

  void OnPublishError(int error_code, const string &error_message) override {
    cout << "------------------OnPublishError error_code:" << error_code
         << ",error_message:" << error_message << endl;
  }

private:
  void OrderByEndianIfNeeded(uint8_t *audio_data, int data_size,
                             int bits_per_sample, bool big_endian) {
    if ((BYTE_ORDER == BIG_ENDIAN && big_endian) ||
        (BYTE_ORDER == LITTLE_ENDIAN && !big_endian)) {
      return;
    }
    // do convert
    if (bits_per_sample == 16) {
      for (int i = 0; i < data_size;) {
        std::swap(audio_data[i], audio_data[i + 1]);
        i += 2;
      }
    } else if (bits_per_sample == 32) {
      for (int i = 0; i < data_size;) {
        std::swap(audio_data[i], audio_data[i + 3]);
        std::swap(audio_data[i + 1], audio_data[i + 2]);
        i += 4;
      }
    }
  }

  void StartCustomVideoPush() {
    if (video_thread_ != nullptr) {
      return;
    }
    video_thread_ = unique_ptr<thread>(new thread([&] {
      FILE *file = fopen(config_.yuv_video.path.c_str(), "r");
      assert(file);
      int yuv_frame_size =
          config_.yuv_video.width * config_.yuv_video.height * 1.5;
      int64_t ts = 0;
      while (published_ && video_track_) {
        auto *yuv_data = (uint8_t *)malloc(yuv_frame_size);
        auto count = fread(yuv_data, sizeof(uint8_t), yuv_frame_size, file);
        if (count < yuv_frame_size || feof(file)) {
          cout << "push video from beginning" << endl;
          free(yuv_data);
          fseek(file, 0, SEEK_SET);
          continue;
        } else {
          video_track_->PushVideoFrame(
              yuv_data, yuv_frame_size, config_.yuv_video.width,
              config_.yuv_video.height, ts, QNVideoFrameType::kI420,
              QNVideoRotation::kVideoRotation0, false);
          int64_t interval_us = 1000000 / config_.yuv_video.fps;
          this_thread::sleep_for(chrono::microseconds(interval_us));
          ts += interval_us;
        }
      };
    }));
  }

  void StartCustomAudioPush() {
    if (audio_thread_ != nullptr) {
      return;
    }
    audio_thread_ = unique_ptr<thread>(new thread([&] {
      FILE *file = fopen(config_.audio.path.c_str(), "r");
      assert(file);
      int pcm_frame_size = config_.audio.sample_rate * config_.audio.channels *
                           config_.audio.bits_per_sample / 8 / 10;
      while (published_ && audio_track_) {
        auto *pcm_data = (uint8_t *)malloc(pcm_frame_size);
        auto count = fread(pcm_data, sizeof(uint8_t), pcm_frame_size, file);
        if (count < pcm_frame_size || feof(file)) {
          cout << "push audio from beginning" << endl;
          free(pcm_data);
          fseek(file, 0, SEEK_SET);
          continue;
        } else {
          OrderByEndianIfNeeded(pcm_data, pcm_frame_size,
                                config_.audio.bits_per_sample,
                                config_.audio.big_endian);
          audio_track_->PushAudioFrame(
              pcm_data, pcm_frame_size, config_.audio.bits_per_sample,
              config_.audio.sample_rate, config_.audio.channels);
          this_thread::sleep_for(chrono::milliseconds(99));
        }
      };
    }));
  }

private:
  QNCustomAudioTrack *audio_track_ = nullptr;
  QNCustomVideoTrack *video_track_ = nullptr;

  bool joined_ = false;
  bool published_ = false;

  unique_ptr<thread> video_thread_;
  unique_ptr<thread> audio_thread_;

  Config config_;
};

int main(int argc, char *argv[]) {
  Config config;
  assert(!config.app.app_id.empty());
  assert(!config.app.room_name.empty());
  assert(!config.yuv_video.path.empty());
  assert(!config.audio.path.empty());

  string version;
  QNRTC::GetVersion(version);
  cout << "SDK 版本:" << version << endl;

  QNRTCSetting setting;
  QNRTC::Init(setting, nullptr);
  QNRTC::SetLogFile(QNLogLevel::kLogInfo, config.app.log_dir,
                    "qn-rtc-demo.log");

  std::string token;
  GetRoomToken_s(config.app.app_id, config.app.room_name, "linux_demo_yuv_pcm_push",
                 "api-demo.qnsdk.com", 10000, token);
  YuvPcmPush client(config);
  client.Join(token, "");
  while (true) {
    cout << "输入 e 停止" << endl;
    string cmd;
    cin >> cmd;
    if (cmd == "e") {
      break;
    } else {
      cout << "未知命令" << endl;
    }
  }
  client.Leave();
  cout << "自定义音视频推流停止" << endl;
}
