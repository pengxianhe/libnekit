cmake_minimum_required(VERSION 3.0.0)

if (CMAKE_VERSION VERSION_LESS "3.7.1")
  message(WARNING "It is highly recommended to used the latest version of CMake.")
  message(WARNING "To support Android, you should use at least 3.7.1")
endif()

project(libnekit C CXX)

set(PLATFORM "" CACHE STRING "Name of the platform.")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag(-Wall HAS_WALL)
if(HAS_WALL)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

check_cxx_compiler_flag(-Wextra HAS_WEXTRA)
if(HAS_WEXTRA)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
endif()

check_cxx_compiler_flag(-pedantic HAS_PEDANTIC)
if(HAS_PEDANTIC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
endif()

option(COVERAGE "Generate coverage report" OFF)
if(COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

include_directories("include")
include_directories("deps/${PLATFORM}/include")
list(APPEND CMAKE_LIBRARY_PATH "deps/${PLATFORM}/lib")

if(APPLE)
  execute_process(COMMAND scripts/find_include_path.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE INCLUDE_DIRS)
  string(REPLACE "\n" ";" INCLUDE_DIRS ${INCLUDE_DIRS})
  include_directories(${INCLUDE_DIRS})
  message(STATUS "Added more include dirs for apple clang")
endif()

if(WIN32)
  add_definitions(-DELPP_WINSOCK2 -DWIN32_LEAN_AND_MEAN)
endif()

set(SOURCES
  src/stream_coder/stream_coder_pipe.cc
  src/stream_coder/socks5_stream_coder_session.cc
  src/utils/buffer.cc
  src/utils/boost_tcp_socket.cc
  src/utils/tcp_socket_interface.cc
  )

add_library(nekit ${SOURCES})
target_compile_features(nekit PUBLIC cxx_lambdas)
target_compile_features(nekit PUBLIC cxx_decltype)
target_compile_features(nekit PUBLIC cxx_auto_type)

find_library(BOOST boost 
  NO_SYSTEM_ENVIROMENT_PATH)
if(NOT BOOST)
  message(FATAL_ERROR "Boost not found. Build dependency first.")
endif()
set(LIBS ${LIBS} ${BOOST})

find_library(EASY_LOGGING easyloggingpp NO_SYSTEM_ENVIROMENT_PATH)
if(NOT EASY_LOGGING)
  message(FATAL_ERROR "Easylogging++ not found. Build dependency first.")
endif()
set(LIBS ${LIBS} ${EASY_LOGGING})

target_link_libraries(nekit ${LIBS})

enable_testing()
add_subdirectory(test)
