//
// Created by phelps-ubuntu on 2022/4/24.
//

#ifndef MEDIA_TOOLBOX_H264_UTIL_H
#define MEDIA_TOOLBOX_H264_UTIL_H

#include <cstdint>
#include <cstdio>

namespace mt {
namespace h264 {

struct StartCodeIndex {
  size_t offset;
  // 3 or 4, 0 invalid
  int size;
};

struct NALUIndex {
  StartCodeIndex start_code_index;
  // include nal type
  size_t payload_size;
};

// h264 寻找起始码
StartCodeIndex FindStartCodeIndex(const uint8_t *data, size_t data_size);
// 寻找一个 NALU
NALUIndex FindNALUIndex(const uint8_t* data, size_t data_size, bool eos);

}
} // namespace mt

#endif // MEDIA_TOOLBOX_H264_UTIL_H
