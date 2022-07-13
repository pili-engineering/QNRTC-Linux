#include "simple_client.h"
#include <cassert>
#include <iostream>
#include <mutex>
#include <csignal>
//#include <execinfo.h>

using namespace std;
using namespace qiniu;

/**
static void SystemErrorHandler(int signum) {
  const int len = 1024;
  void *func[len];
  size_t size;
  int i;
  char **funs;

  signal(signum, SIG_DFL);
  size = backtrace(func, len);
  funs = (char **)backtrace_symbols(func, size);
  fprintf(stderr, "System error, Stack trace:\n");
  for (i = 0; i < size; ++i)
    fprintf(stderr, "%d %s \n", i, funs[i]);
  free(funs);
  // exit(1);
}
*/

SimpleClient::SimpleClient() {
 // signal(SIGSEGV, SystemErrorHandler);
 // signal(SIGABRT, SystemErrorHandler);
  // 创建 Client 用于音视频会话
  client_ = QNRTC::CreateClient(this);
  // 自动订阅远端音视频
  client_->SetAutoSubscribe(false);
}

SimpleClient::~SimpleClient() {
  QNRTC::DestroyClient(client_);
 // signal(SIGSEGV, SystemErrorHandler);
 // signal(SIGABRT, SystemErrorHandler);
}

void SimpleClient::OnConnectionStateChanged(
    QNConnectionState state, const QNConnectionDisconnectedInfo *info) {
  cout << "------------------OnConnectionStateChanged state:" << state << endl;
}

void SimpleClient::OnUserJoined(const string &remote_user_id,
                                const string &user_data) {
  cout << "------------------OnUserJoined user:" << remote_user_id << endl;
}

void SimpleClient::OnUserLeft(const string &remote_user_id) {
  cout << "------------------OnUserLeft user:" << remote_user_id << endl;
}

void SimpleClient::OnUserReconnecting(const string &remote_user_id) {
  cout << "------------------OnUserReconnecting user:" << remote_user_id
       << endl;
}

void SimpleClient::OnUserReconnected(const string &remote_user_id) {
  cout << "------------------OnUserReconnected user:" << remote_user_id << endl;
}

void SimpleClient::OnUserPublished(const string &remote_user_id,
                                   const RemoteTrackList &track_list) {
  cout << "------------------OnUserPublished user:" << remote_user_id << endl;
}

void SimpleClient::OnUserUnpublished(const string &remote_user_id,
                                     const RemoteTrackList &track_list) {
  cout << "------------------OnUserUnpublished user:" << remote_user_id << endl;
}

void SimpleClient::OnSubscribed(
    const string &remote_user_id,
    const RemoteAudioTrackList &remote_audio_track_list,
    const RemoteVideoTrackList &remote_video_track_list) {
  cout << "------------------OnSubscribed user:" << remote_user_id << endl;
}

void SimpleClient::OnMessageReceived(const QNCustomMessage &message) {
  cout << "------------------OnMessageReceived message:" << message.msg_text
       << endl;
}

void SimpleClient::OnMediaRelayStateChanged(const std::string &relay_room,
                                            const QNMediaRelayState state) {
  cout << "------------------OnMessageReceived relay room:" << relay_room
       << endl;
}
