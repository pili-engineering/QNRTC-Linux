#include "simple_client.h"
#include "../config.h"
#include <cassert>
#include <endian.h>
#include <iostream>
#include <thread>

using namespace std;
using namespace qiniu;

class CdnStreaming : public SimpleClient,
                     public qiniu::QNPublishResultCallback,
                     public QNLiveStreamingListener {

public:
  void Join(string token, string user_data) override {
    if (joined_) {
      return;
    }
    client_->SetAutoSubscribe(false);
    client_->SetLiveStreamingListener(this);
    client_->Join(token, user_data);
  }

  void Leave() override {
    if (joined_) {
//      QNRTC::DestroyLocalTrack(audio_track_);
//      QNRTC::DestroyLocalTrack(video_track_);
      published_ = false;
      client_->Leave();
      joined_ = false;
    }
  }

  void StartDirectStreaming() {
    QNDirectLiveStreamingConfig config;
    config.local_audio_track = audio_track_;
    config.local_video_track = video_track_;
    config.publish_url = "rtmp://pili-publish.qnsdk.com/sdk-live/linux-forward";
    config.stream_id = "linux-forward-id";
    client_->StartLiveStreaming(config);
  }

  void StopDirectStreaming() {
    QNDirectLiveStreamingConfig config;
    config.stream_id = "linux-forward-id";
    client_->StopLiveStreaming(config);
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
      // microphone audio track
      QNMicrophoneAudioTrackConfig audio_track_config;
      audio_track_config.audio_quality = {44100, 2, 16, 44100};
      audio_track_ = QNRTC::CreateMicrophoneAudioTrack(audio_track_config);
      local_track_list.push_front(audio_track_);

      // camera video track
      // select camera
      int count = QNRTC::GetCameraCount();
      assert(count != 0);
      QNCameraInfo target_camera;
      int target_cap_size = 0;
      for (int i = 0; i < count; ++i) {
        auto camera_info = QNRTC::GetCameraInfo(i);
        if (camera_info.capabilities.size() > target_cap_size) {
          target_camera = camera_info;
        }
      }
      QNCameraCapability target_camera_cap;
      for (int i = 0; i < target_camera.capabilities.size(); ++i) {
        if (target_camera.capabilities[i].video_frame_type ==
            QNVideoFrameType::kI420) {
          target_camera_cap = target_camera.capabilities[i];
          break;
        }
      }
      auto cap = target_camera_cap;
      cout << cap.width << "," << cap.height << "," << cap.max_fps << ","
           << cap.video_frame_type << endl;
      QNCameraVideoTrackConfig video_track_config;
      video_track_config.id = target_camera.id;
      video_track_config.capture_config = {cap.width, cap.height, cap.max_fps};
      video_track_config.encoder_config = {cap.width, cap.height, cap.max_fps,
                                           cap.width * cap.height * 2};
      video_track_ = QNRTC::CreateCameraVideoTrack(video_track_config);
      local_track_list.push_front(video_track_);

      client_->Publish(local_track_list, this);
    } else {
      joined_ = false;
    }
  }

  void OnPublished() override {
    cout << "------------------OnPublished" << endl;
    published_ = true;
  }

  void OnPublishError(int error_code, const string &error_message) override {
    cout << "------------------OnPublishError error_code:" << error_code
         << ",error_message:" << error_message << endl;
    published_ = false;
  }

  /**
   * 转推任务成功创建时触发此回调
   *
   * @param stream_id 转推成功的 stream id
   */
  void OnStarted(const std::string &stream_id) override {
    cout << "streaming started: " << stream_id << endl;
  }

  /**
   * 转推任务成功停止时触发此回调
   *
   * @param stream_id 停止转推的 stream id
   */
  void OnStopped(const std::string &stream_id) override {
    cout << "streaming stopped: " << stream_id << endl;
  }

  /**
   * 转推任务配置更新时触发此回调
   *
   * @param stream_id 配置更新的 stream id
   */
  void OnTranscodingTracksUpdated(const std::string &stream_id) override {
    cout << "streaming updated: " << stream_id << endl;
  }

  /**
   * 转推任务出错时触发此回调
   * @param stream_id 出现错误的 stream id
   * @param error_info 详细错误原因
   */
  void
  OnLiveStreamingError(const std::string &stream_id,
                       const QNLiveStreamingErrorInfo &error_info) override {
    cout << "streaming error: " << stream_id << endl;
  }

private:
  QNMicrophoneAudioTrack *audio_track_ = nullptr;
  QNCameraVideoTrack *video_track_ = nullptr;

  bool joined_ = false;
  bool published_ = false;
};

void print_cmd() {
  cout << "|------------------------------------------|" << endl;
  cout << "| 请输入命令：                             |" << endl;
  cout << "| start:                          开始推流 |" << endl;
  cout << "| stop:                           停止推流 |" << endl;
  cout << "| e:                               退出   |" << endl;
  cout << "|------------------------------------------|" << endl;
}

int main(int argc, char *argv[]) {
  Config config;
  assert(!config.app.app_id.empty());
  assert(!config.app.room_name.empty());

  string version;
  QNRTC::GetVersion(version);
  cout << "SDK 版本:" << version << endl;

  QNRTCSetting setting;
  QNRTC::Init(setting, nullptr);
  QNRTC::SetLogFile(QNLogLevel::kLogInfo, config.app.log_dir,
                    "qn-rtc-demo.log");

  std::string token;
  GetRoomToken_s(config.app.app_id, config.app.room_name, "linux_demo_streaming",
                 "api-demo.qnsdk.com", 10000, token);
  CdnStreaming client;
  client.Join(token, "");
  print_cmd();

  while (true) {
    string cmd;
    cin >> cmd;
    if (cmd == "start") {
      client.StartDirectStreaming();
    } else if (cmd == "stop") {
      client.StopDirectStreaming();
    } else if (cmd == "e") {
      client.Leave();
      QNRTC::DeInint();
      this_thread::sleep_for(chrono::milliseconds(1000));
      cout << "exited" << endl;
      return 0;
    } else {
      cout << "未知命令" << endl;
      print_cmd();
    }
  }
}
