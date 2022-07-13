cmake_minimum_required(VERSION 3.4.1)
project(qn-rtc-demo)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -g -std=c++11")

include_directories(${CMAKE_CURRENT_LIST_DIR}/common)
include_directories(${SDK_INCLUDE_PATH})
link_directories(${SDK_LIB_PATH})

set(COMMON_LIBS qn_rtc
  pthread
  dl
  m)

# video talk
add_executable(video_talk 
  ${CMAKE_CURRENT_LIST_DIR}/video_talk/video_talk.cc
  ${CMAKE_CURRENT_LIST_DIR}/common/simple_client.cc
  )

target_link_libraries(
  video_talk
  PRIVATE
  ${COMMON_LIBS}
  )

# yuv pcm push
add_executable(yuv_pcm_push 
  ${CMAKE_CURRENT_LIST_DIR}/yuv_pcm_push/yuv_pcm_push.cc
  ${CMAKE_CURRENT_LIST_DIR}/common/simple_client.cc
  )

target_link_libraries(
  yuv_pcm_push
  PRIVATE
  ${COMMON_LIBS}
  )

# cdn streaming
add_executable(cdn_streaming 
  ${CMAKE_CURRENT_LIST_DIR}/cdn_streaming/cdn_streaming.cc
  ${CMAKE_CURRENT_LIST_DIR}/common/simple_client.cc
  )

target_link_libraries(
  cdn_streaming
  PRIVATE
  ${COMMON_LIBS}
  )

# h264 pcm push
add_executable(h264_pcm_push
        ${CMAKE_CURRENT_LIST_DIR}/h264_pcm_push/h264_pcm_push.cc
        ${CMAKE_CURRENT_LIST_DIR}/h264_pcm_push/h264_file_parser.cc
        ${CMAKE_CURRENT_LIST_DIR}/h264_pcm_push/h264_util.cc
        ${CMAKE_CURRENT_LIST_DIR}/common/simple_client.cc
        )

target_link_libraries(
        h264_pcm_push
        PRIVATE
        ${COMMON_LIBS}
)
