//
// Created by phelps-ubuntu on 2022/4/21.
//

#ifndef QNRTCENGINE_H264_HELPER_H
#define QNRTCENGINE_H264_HELPER_H

#include <functional>
#include "h264_util.h"


static const uint8_t *avc_find_startcode_internal(const uint8_t *p,
                                                  const uint8_t *end) {
  const uint8_t *a = p + 4 - ((intptr_t)p & 3);

  for (end -= 3; p < a && p < end; p++) {
    if (p[0] == 0 && p[1] == 0 && p[2] == 1)
      return p;
  }

  for (end -= 3; p < end; p += 4) {
    uint32_t x = *(const uint32_t *)p;
    //      if ((x - 0x01000100) & (~x) & 0x80008000) // little endian
    //      if ((x - 0x00010001) & (~x) & 0x00800080) // big endian
    if ((x - 0x01010101) & (~x) & 0x80808080) { // generic
      if (p[1] == 0) {
        if (p[0] == 0 && p[2] == 1)
          return p;
        if (p[2] == 0 && p[3] == 1)
          return p + 1;
      }
      if (p[3] == 0) {
        if (p[2] == 0 && p[4] == 1)
          return p + 2;
        if (p[4] == 0 && p[5] == 1)
          return p + 3;
      }
    }
  }

  for (end += 3; p < end; p++) {
    if (p[0] == 0 && p[1] == 0 && p[2] == 1)
      return p;
  }

  return end + 3;
}

const uint8_t *ff_avc_find_startcode(const uint8_t *p, const uint8_t *end){
  const uint8_t *out = avc_find_startcode_internal(p, end);
  if(p<out && out<end && !out[-1]) out--;
  return out;
}

 static void h264_frame_split(const unsigned char *buf_in, int size,
 const std::function<void(const unsigned char *nalu, unsigned len, int is_last)>& cb)
 {
   const unsigned char *p = buf_in;
   const unsigned char *end = p + size;
   const unsigned char *nal_start, *nal_end;

   size = 0;
   nal_start = ff_avc_find_startcode(p, end);
   for (;;) {
     while (nal_start < end && !*(nal_start++));
     if (nal_start == end)
       break;
     // 找到一个 nalu
     nal_end = ff_avc_find_startcode(nal_start, end);
     cb(nal_start, nal_end - nal_start, 0);
//     NALU *tmp = av_fast_realloc(list->nalus, &list->nalus_array_size,
//                                 (list->nb_nalus + 1) * sizeof(*list->nalus));
//     if (!tmp)
//       return AVERROR(ENOMEM);
//     list->nalus = tmp;
//     tmp[list->nb_nalus++] = (NALU){ .offset = nal_start - p,
//                                    .size   = nal_end - nal_start };

     size += 4 + nal_end - nal_start;
     nal_start = nal_end;
   };
}

static void h264_frame_split_mt(const unsigned char *buf_in, int size,
                             const std::function<void(const mt::h264::NALUIndex& nalu_index)>& cb)
{
  size_t pos = 0;
  while (pos < size) {
    auto nalu_index = mt::h264::FindNALUIndex(buf_in + pos, size- pos, true);
    if (nalu_index.payload_size != 0) {
      // 需要补齐 pos
      nalu_index.start_code_index.offset += pos;
      cb(nalu_index);
      pos += nalu_index.start_code_index.size;
      pos += nalu_index.payload_size;
    } else {
      break;
    }
  }
}

//static void h264_frame_split(
//    const unsigned char *frame, int len,
//    std::function<void(const unsigned char *nalu, unsigned len, int is_last)>
//        cb) {
//  const unsigned char *byte;
//  unsigned int next4bytes;
//  const unsigned char *nalu1_begin;
//  const unsigned char *nalu2_begin;
//
//  /* H264白皮书 B.1.2 节对 NALU 语义结构说明如下
//   *
//   * leading_zero_8bits: 0x00 当 NALU 为字节流的第一个 NALU 时包含
//   * zero_byte: 0x00 当 NALU 为 SPS/PPS, 或为 Access Unit 的第一个 NALU 时包含
//   * start_code_prefix_one_3bytes: 0x000001, NALU 起始码前缀
//   *  < 具体的 NALU 数据 >
//   * trailing_zero_8bits: 0x00
//   *
//   * 综上述条件，可以看出具体的 NALU 数据是被 0x00000001 所分割的，或额外包含 0
//   * 字节 下述分割过程既是基于上述结构来进行的
//   */
//
//  byte = frame;
//  while ((byte + 4) < (frame + len)) {
//    next4bytes = ((unsigned int)byte[0] << 24) | ((unsigned int)byte[1] << 16) |
//                 ((unsigned int)byte[2] << 8) | (unsigned int)byte[3];
//
//    // 不等与 start code 则继续往下读
//    if (next4bytes != 0x00000001) {
//      byte++;
//      continue;
//    }
//
//    /* 跳过自身的 start_code_prefix_one_3bytes，以及 leading_zero_8bits 或前一个
//     * NALU 的 trailing_zero_8bits */
//    nalu1_begin = byte + 4;
//
//    nalu2_begin = nalu1_begin + 1; /* 跳过nalu_type字节 */
//    while ((nalu2_begin + 4) < (frame + len)) {
//      next4bytes = ((unsigned int)nalu2_begin[0] << 24) |
//                   ((unsigned int)nalu2_begin[1] << 16) |
//                   ((unsigned int)nalu2_begin[2] << 8) |
//                   (unsigned int)nalu2_begin[3];
//      if (next4bytes == 0x00000001) {
//        break;
//      }
//
//      nalu2_begin++;
//    }
//
//    if ((nalu2_begin + 4) == (frame + len)) {
//      /* nalu1_begin 指向的 NALU 已经是当前帧中的最后一个 NALU */
//
//      if (next4bytes == 0x00000001) {
//        cb(nalu1_begin - 4, (nalu2_begin - nalu1_begin) + 4, 1);
//        //        nalu_sink(nalu1_begin, (nalu2_begin - nalu1_begin), 1,
//        //        userdata);
//      } else {
//        cb(nalu1_begin - 4, (frame + len - nalu1_begin) + 4, 1);
//        //        nalu_sink(nalu1_begin, (frame + len - nalu1_begin), 1,
//        //        userdata);
//      }
//
//      break;
//    } else {
//      /* nalu2_begin 是指向下一个nalu的start_code_prefix */
//      byte = nalu2_begin;
//      cb(nalu1_begin - 4, (nalu2_begin - nalu1_begin) + 4, 0);
//      //      nalu_sink(nalu1_begin, (nalu2_begin - nalu1_begin), 0, userdata);
//    }
//  }
//}

#endif // QNRTCENGINE_H264_HELPER_H
