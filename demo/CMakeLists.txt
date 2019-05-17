cmake_minimum_required(VERSION 3.5)
project(demo)

set(CMAKE_POSITION_INDEPENDENT_CODE     TRUE)
set(C_CXX_FLAGS                         "-std=c++11")
set(CMAKE_C_FLAGS                       "-fPIC -Wno-unused-variable ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS                     "-fPIC -std=c++11 -Wno-unused-variable ${CMAKE_CXX_FLAGS}")
#set(CMAKE_EXE_LINKER_FLAGS              "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS              "${CMAKE_EXE_LINKER_FLAGS} -g")

set(CMAKE_BUILD_TYPE Debug)

if (Linux)
set(ARCH linux)
elseif (Arm)
set(ARCH arm)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../Release/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/curl/install/${ARCH}/include
)

set(SOURCE_FILES
    main.c
    util.c
    callback.c
)

link_directories(
    ${link_directories}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Release/lib/${ARCH}
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/curl/install/${ARCH}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/boringssl/install/${ARCH}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/boringssl/install/${ARCH}/lib
)

link_libraries(
    "libqn_rtc.so"
    "libcurl.so"
    "libssl.so"
    "libcrypto.so"
)

add_executable(demo ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(demo pthread)

set_target_properties(demo PROPERTIES OUTPUT_NAME "demo")
