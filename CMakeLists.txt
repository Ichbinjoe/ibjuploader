cmake_minimum_required(VERSION 3.2)
project(ibjuploader)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lcurl")

set(SOURCE_FILES src/main.c src/buffer.h src/buffer.c)
add_executable(ibjuploader ${SOURCE_FILES})
target_link_libraries(ibjuploader curl)
target_link_libraries(ibjuploader magic)

set(EXECUTABLE_OUTPUT_PATH bin/)