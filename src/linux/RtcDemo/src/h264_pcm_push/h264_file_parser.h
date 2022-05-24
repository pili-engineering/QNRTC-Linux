//
// Created by phelps-ubuntu on 2022/4/25.
//

#ifndef MEDIA_TOOLBOX_H264_FILE_PARSER_H
#define MEDIA_TOOLBOX_H264_FILE_PARSER_H

#include "h264_util.h"

#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace mt {
namespace h264 {

struct NALUData {
  const uint8_t *data = nullptr;
  int start_code_size = 0;
  int payload_size = 0;
};

class H264FileParser {
public:
  explicit H264FileParser(std::string path);
  ~H264FileParser();

  bool Init();
  void DeInit();

  const NALUData *ReadNALU();
  bool Advance();

private:
  bool FindNALUFromBuffer();
  void UpdateBuffer();

private:
  std::string input_file_path_;
  std::unique_ptr<std::ifstream> input_file_;
  bool input_eof_;

  std::vector<uint8_t> input_buffer_;
  size_t input_buffer_pos_;
  size_t input_buffer_remaining_;

  NALUData current_nalu_data_;

  bool inited_;

  static size_t kDefaultInputBufferSize;
};
} // namespace h264
} // namespace mt

#endif // MEDIA_TOOLBOX_H264_FILE_PARSER_H
