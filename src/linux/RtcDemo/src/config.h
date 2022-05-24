#pragma once

#include <string>
#include <cassert>
#include <fstream>
#include "qn_common_def.h"

struct AppConfig {
  std::string app_id = "g2m0ya7w7";
  std::string room_name = "linux_demo_room";
  std::string log_dir = "/home/tmp";
};

struct AudioConfig {
  std::string path = "/home/phelps-ubuntu/workspace/qiniu/pili-rtc-pc-kit/src/linux/RtcDemo/resources/44100_16bits_2channels_little.pcm";
  int sample_rate = 44100;
  int bits_per_sample = 16;
  int channels = 2;
  bool big_endian = false;
};

struct YUVVideoConfig {
  std::string path = "/home/phelps-ubuntu/workspace/qiniu/pili-rtc-pc-kit/src/linux/RtcDemo/resources/426x240.yuv";
  int width = 426;
  int height = 240;
  int fps = 30;
};

struct H264VideoConfig {
  std::string path = "/home/phelps-ubuntu/Downloads/test/test.h264";
  int width = 1280;
  int height = 720;
  int fps = 60;
};

struct Config {
  YUVVideoConfig yuv_video;
  H264VideoConfig h264_video;
  AudioConfig audio;
  AppConfig app;
};