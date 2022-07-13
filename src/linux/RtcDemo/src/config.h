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
  std::string path = "/home/phelps/Workspace/qiniu/pili-rtc-pc-kit/Resources/44100_16bits_2channels_little.pcm";
  int sample_rate = 44100;
  int bits_per_sample = 16;
  int channels = 2;
  bool big_endian = false;
};

struct YUVVideoConfig {
  std::string path = "/home/phelps/Workspace/qiniu/pili-rtc-pc-kit/Resources/426x240_25.yuv";
  int width = 426;
  int height = 240;
  int fps = 25;
};

struct H264VideoConfig {
  std::string path = "/home/phelps/Workspace/qiniu/pili-rtc-pc-kit/Resources/540x960_30.h264";
  int width = 540;
  int height = 960;
  int fps = 30;
};

struct Config {
  YUVVideoConfig yuv_video;
  H264VideoConfig h264_video;
  AudioConfig audio;
  AppConfig app;
};
