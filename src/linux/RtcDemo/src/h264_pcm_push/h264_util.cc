//
// Created by phelps-ubuntu on 2022/4/24.
//

#include "h264_util.h"
#include <cassert>
#include <cstring>

namespace mt {
namespace h264 {

StartCodeIndex FindStartCodeIndex(const uint8_t *buffer, size_t buffer_size) {
  if (buffer_size < 3) {
    return {0, 0};
  }

  const size_t end = buffer_size - 3;
  for (size_t i = 0; i < end;) {
    if (buffer[i + 2] > 1) {
      i += 3;
    } else if (buffer[i + 2] == 1 && buffer[i + 1] == 0 && buffer[i] == 0) {
      if (i > 0 && buffer[i - 1] == 0) {
        return {i-1, 4};
      } else {
        return {i, 3};
      }
    } else {
      ++i;
    }
  }
  return {0, 0};
}

NALUIndex FindNALUIndex(const uint8_t *buffer, size_t buffer_size, bool eos) {
  if (buffer_size < 3) {
    return {{0, 0}, 0};
  }

  // 寻找第一个 start code
  auto start_code_index = FindStartCodeIndex(buffer, buffer_size);
  if (start_code_index.size == 0) {
    // 找不到 start code
    return {{0, 0}, 0};
  }
  // 寻找下一个 start code
  auto end_code_index = FindStartCodeIndex(buffer + start_code_index.offset + start_code_index.size,
                                           buffer_size - start_code_index.offset - start_code_index.size);
  if (end_code_index.size == 0) {
    // 未找到结束位置
    if (eos) {
      return {start_code_index, buffer_size - start_code_index.offset - start_code_index.size};
    } else {
      return {{0, 0}, 0};
    }
  } else {
    return {start_code_index, end_code_index.offset};
  }
}


} // namespace h264
} // namespace mt