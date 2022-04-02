#include "json.hpp"
#include "simple_client.h"
#include <cassert>
#include <endian.h>
#include <iostream>
#include <thread>

using namespace std;
using namespace qiniu;

struct AudioInfo {
  string path;
  int sample_rate;
  int bits_per_sample;
  int channels;
  bool big_endian;
};

struct VideoInfo {
  string path;
  string type;
  int width;
  int height;
  int fps;
};

class YuvPcmPush : public SimpleClient, public qiniu::QNPublishResultCallback {

public:
  explicit YuvPcmPush(VideoInfo &video_info, AudioInfo &audio_info)
      : video_info_(video_info), audio_info_(audio_info) {}

  ~YuvPcmPush() {}

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
          audio_info_.sample_rate, audio_info_.channels,
          audio_info_.bits_per_sample, audio_info_.sample_rate};
      audio_track_ = QNRTC::CreateCustomAudioTrack(custom_audio_track_config);
      local_track_list.push_front(audio_track_);

      // custom video track
      QNCustomVideoTrackConfig custom_video_track_config;
      custom_video_track_config.encoder_config = {
          video_info_.width, video_info_.height, video_info_.fps,
          video_info_.width * video_info_.height * 2};
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
    video_thread_ = make_unique<thread>([&] {
      FILE *file = fopen(video_info_.path.c_str(), "r");
      assert(file);
      int yuv_frame_size = video_info_.width * video_info_.height * 1.5;
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
          video_track_->PushVideoFrame(yuv_data, yuv_frame_size,
                                       video_info_.width, video_info_.height,
                                       ts, QNVideoFrameType::kI420,
                                       QNVideoRotation::kVideoRotation0, false);
          int64_t interval_us = 1000000 / video_info_.fps;
          this_thread::sleep_for(chrono::microseconds(interval_us));
          ts += interval_us;
        }
      };
    });
  }

  void StartCustomAudioPush() {
    if (audio_thread_ != nullptr) {
      return;
    }
    audio_thread_ = make_unique<thread>([&] {
      FILE *file = fopen(audio_info_.path.c_str(), "r");
      assert(file);
      int pcm_frame_size = audio_info_.sample_rate * audio_info_.channels *
                           audio_info_.bits_per_sample / 8 / 10;
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
                                audio_info_.bits_per_sample,
                                audio_info_.big_endian);
          audio_track_->PushAudioFrame(
              pcm_data, pcm_frame_size, audio_info_.bits_per_sample,
              audio_info_.sample_rate, audio_info_.channels);
          this_thread::sleep_for(chrono::milliseconds(99));
        }
      };
    });
  }

private:
  QNCustomAudioTrack *audio_track_ = nullptr;
  QNCustomVideoTrack *video_track_ = nullptr;

  bool joined_ = false;
  bool published_ = false;

  unique_ptr<thread> video_thread_;
  unique_ptr<thread> audio_thread_;

  VideoInfo video_info_;
  AudioInfo audio_info_;
};

int main(int argc, char *argv[]) {
  string version;
  QNRTC::GetVersion(version);
  cout << "SDK 版本:" << version << endl;

  QNRTCSetting setting;
  QNRTC::Init(setting, nullptr);
  QNRTC::SetLogFile(QNLogLevel::kLogInfo,
  "/home/phelps-ubuntu/workspace/qiniu/pili-rtc-pc-kit/src/linux/RtcDemo",
  "qn-rtc-demo.log");

  auto config_file_path = "/home/phelps-ubuntu/workspace/qiniu/pili-rtc-pc-kit/"
                          "src/linux/RtcDemo/src/yuv_pcm_push/config.json";

  FILE *file = fopen(config_file_path, "r");
  assert(file);
  string json_file;
  json_file.resize(10000);
  int size = fread((void *)json_file.data(), sizeof(char), 10000, file);
  json_file.resize(size);
  auto custom_info_json = nlohmann::json::parse(json_file);
  cout << custom_info_json << endl;
  nlohmann::json &video_info_json = custom_info_json["video"];
  nlohmann::json &audio_info_json = custom_info_json["audio"];
  VideoInfo video_info{
      video_info_json["path"],  video_info_json["type"],
      video_info_json["width"], video_info_json["height"],
      video_info_json["fps"],
  };
  AudioInfo audio_info{
      audio_info_json["path"],
      audio_info_json["sample_rate"],
      audio_info_json["bits_per_sample"],
      audio_info_json["channels"],
      audio_info_json["big_endian"],
  };

  std::string token;
  GetRoomToken_s("d8lk7l4ed", "linux_sdk", "linux_demo", "api-demo.qnsdk.com",
                 100000, token);
  YuvPcmPush client(video_info, audio_info);
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
