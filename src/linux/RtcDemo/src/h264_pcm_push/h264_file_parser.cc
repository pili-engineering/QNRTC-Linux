//
// Created by phelps-ubuntu on 2022/4/25.
//

#include "h264_file_parser.h"

#include <cassert>
#include <cstring>
#include <utility>

namespace mt {
namespace h264 {

size_t H264FileParser::kDefaultInputBufferSize = 1024 * 1024 * 100;

H264FileParser::H264FileParser(std::string path)
    : input_file_path_(std::move(path)), input_eof_(false), inited_(false),
      input_buffer_pos_(0), input_buffer_remaining_(0) {}

H264FileParser::~H264FileParser() = default;

bool H264FileParser::Init() {
  current_nalu_data_.start_code_size = 0;
  current_nalu_data_.payload_size = 0;

  input_buffer_.resize(kDefaultInputBufferSize);
  input_buffer_remaining_ = 0;
  input_buffer_pos_ = 0;

  input_file_ = std::unique_ptr<std::ifstream>(new std::ifstream(input_file_path_));
  input_eof_ = false;
  inited_ = input_file_ != nullptr && input_file_->is_open();
  return inited_;
}

void H264FileParser::DeInit() {
  input_file_.reset(nullptr);
  inited_ = false;
}

const NALUData *H264FileParser::ReadNALU() {
  assert(inited_);
  // for first time
  if (current_nalu_data_.payload_size == 0) {
    Advance();
  }
  return &current_nalu_data_;
}

bool H264FileParser::Advance() {
  while (true) {
    // 尝试从已有的缓存解析 nalu
    if (FindNALUFromBuffer()) {
      return true;
    }
    if (input_eof_) {
      return false;
    }
    // 读取部分文件到缓存
    UpdateBuffer();
  }
}

bool H264FileParser::FindNALUFromBuffer() {
  if (input_buffer_remaining_ < 3) {
    current_nalu_data_.payload_size = 0;
    return false;
  }
  auto nalu_index = FindNALUIndex(input_buffer_.data() + input_buffer_pos_,
                                  input_buffer_remaining_, input_eof_);
  if (nalu_index.payload_size > 0) {
    // update nalu data
    current_nalu_data_.start_code_size = nalu_index.start_code_index.size;
    current_nalu_data_.payload_size = nalu_index.payload_size;
    current_nalu_data_.data = input_buffer_.data() + input_buffer_pos_ +
                               nalu_index.start_code_index.offset;

    input_buffer_pos_ += (current_nalu_data_.start_code_size +
                          current_nalu_data_.payload_size);
    input_buffer_remaining_ -= (current_nalu_data_.start_code_size +
                                current_nalu_data_.payload_size);
    return true;
  }
  return false;
}

void H264FileParser::UpdateBuffer() {
  // reset current nalu data
  current_nalu_data_.payload_size = 0;
  current_nalu_data_.start_code_size = 0;
  // compact input buffer
  if (input_buffer_pos_ > 0 && input_buffer_remaining_ > 0) {
    std::copy(input_buffer_.begin() + input_buffer_pos_,
              input_buffer_.begin() + input_buffer_pos_ +
                  input_buffer_remaining_,
              input_buffer_.begin());
    input_buffer_pos_ = 0;
  }
  // read
  int chunk_size = 1024 * 4;
  if (input_buffer_.capacity() - input_buffer_remaining_ <= chunk_size) {
    // enlarge
    input_buffer_.resize(input_buffer_.capacity() + chunk_size);
  }
  input_file_->read((char *)input_buffer_.data() + input_buffer_remaining_,
                    chunk_size);
  auto read_count = input_file_->gcount();
  input_buffer_remaining_ += read_count;
  if (input_file_->eof()) {
    input_eof_ = true;
  }
}

} // namespace h264
} // namespace mt