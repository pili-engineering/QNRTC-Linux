#pragma once
#include "qn_rtc_interface.h"

extern "C" QINIU_EXPORT_DLL int
GetRoomToken_s(const std::string &app_id_, const std::string &room_name_,
               const std::string &user_id_, const std::string &host_name_,
               const int time_out_, std::string &token_);



class SimpleClient : public qiniu::QNClientEventListener,
                     public qiniu::QNRTCEventListener {
public:
  explicit SimpleClient();
  ~SimpleClient();

  virtual void Join(std::string token, std::string user_data) = 0;
  virtual void Leave() = 0;

  // QNRTCEventListener
protected:
  void OnConnectionStateChanged(
      qiniu::QNConnectionState state,
      const qiniu::QNConnectionDisconnectedInfo *info) override;
  void OnUserJoined(const std::string &remote_user_id,
                    const std::string &user_data) override;
  void OnUserLeft(const std::string &remote_user_id) override;
  void OnUserReconnecting(const std::string &remote_user_id) override;
  void OnUserReconnected(const std::string &remote_user_id) override;
  void OnUserPublished(const std::string &remote_user_id,
                       const qiniu::RemoteTrackList &track_list) override;
  void OnUserUnpublished(const std::string &remote_user_id,
                         const qiniu::RemoteTrackList &track_list) override;
  void OnSubscribed(
      const std::string &remote_user_id,
      const qiniu::RemoteAudioTrackList &remote_audio_track_list,
      const qiniu::RemoteVideoTrackList &remote_video_track_list) override;
  void OnMessageReceived(const qiniu::QNCustomMessage &message) override;
  void OnMediaRelayStateChanged(const std::string &relay_room,
                                const qiniu::QNMediaRelayState state) override;

protected:
  qiniu::QNRTCClient *client_ = nullptr;
};

