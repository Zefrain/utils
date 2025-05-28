cmake_minimum_required(VERSION 3.10)
project(utils C)

enable_testing()
include(CTest)

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 包含头文件路径
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/gtgo/include
)

# 找出平台对应的 gtgo.a 路径
# WARNING: 虽然可以通过编译, 但是本项目的功能实现仅支持Linux
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(GTGO_LIB_PATH ${PROJECT_SOURCE_DIR}/gtgo/linux/libgtgo_arm64.a)
  else()
    set(GTGO_LIB_PATH ${PROJECT_SOURCE_DIR}/gtgo/linux/libgtgo.a)
  endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(GTGO_LIB_PATH ${PROJECT_SOURCE_DIR}/gtgo/gtgo.xcframework/macos-arm64_x86_64/libgtgo.a)
elseif(WIN32)
  set(GTGO_LIB_PATH ${PROJECT_SOURCE_DIR}/gtgo/windows/libgtgo.a)
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
endif()

# 编译 utils.c 并打包成静态库，链接 gtgo 的对象文件
add_library(utils STATIC src/utils.c)
target_link_libraries(utils PRIVATE ${GTGO_LIB_PATH})

# 使用 find_package 找到 libcurl
set(CURL_USE_STATIC_LIBS ON)
find_package(CURL REQUIRED)
if (CURL_FOUND)
  message(STATUS "Found libcurl: ${CURL_VERSION}")
  # 设置 libcurl 的链接选项
  target_link_libraries(utils PRIVATE ${CURL_LIBRARIES})
  target_include_directories(utils PRIVATE ${CURL_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "libcurl not found")
endif()

# 编译 test_utils，链接 utils（已包含 gtgo）
add_executable(test_utils tests/test_utils.c)
target_link_libraries(test_utils utils)

add_test(NAME test_utils_with_server
  COMMAND /bin/sh -c "
    cd ${CMAKE_SOURCE_DIR} && \
    go run server.go > /dev/null 2>&1 & \
    SERVER_PID=$! && \
    sleep 3 && \
    ${CMAKE_BINARY_DIR}/bin/test_utils; \
    TEST_RESULT=\$?; \
    kill \$SERVER_PID; \
    wait \$SERVER_PID 2>/dev/null; \
    exit \$TEST_RESULT"
)
