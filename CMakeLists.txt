cmake_minimum_required(VERSION 3.31)
project(shan-shui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_library(sscore STATIC
  src/core/rng.cpp
  src/core/noise.cpp
  src/core/poly.cpp
  src/core/util.cpp
  src/core/scratch.cpp
  src/draw/painter.cpp
  src/gen/tree.cpp
  src/gen/man.cpp
  src/gen/arch.cpp
  src/gen/water.cpp
  src/gen/mount.cpp
  src/scene/scene.cpp
)
target_include_directories(sscore PUBLIC src)

if(EMSCRIPTEN)
  # --- WASM module (normal build: ss.js + ss.wasm) ---
  add_executable(ss_web src/api.cpp)
  target_link_libraries(ss_web PRIVATE sscore embind)
  set_target_properties(ss_web PROPERTIES OUTPUT_NAME "ss" SUFFIX ".js")
  target_compile_options(ss_web PRIVATE -O3)
  target_link_options(ss_web PRIVATE
    -O3
    -sMODULARIZE=1
    -sEXPORT_NAME=createSS
    -sALLOW_MEMORY_GROWTH=1
    -sENVIRONMENT=web,node
    -sEXPORTED_RUNTIME_METHODS=ccall,cwrap
  )
  add_custom_command(TARGET ss_web POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/web $<TARGET_FILE_DIR:ss_web>)

  # --- Single-file variant (wasm embedded in ss.js, works from file://) ---
  add_executable(ss_single src/api.cpp)
  target_link_libraries(ss_single PRIVATE sscore embind)
  set_target_properties(ss_single PROPERTIES
    OUTPUT_NAME "ss" SUFFIX ".js"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/single)
  target_compile_options(ss_single PRIVATE -O3)
  target_link_options(ss_single PRIVATE
    -O3
    -sMODULARIZE=1
    -sEXPORT_NAME=createSS
    -sALLOW_MEMORY_GROWTH=1
    -sENVIRONMENT=web,node
    -sSINGLE_FILE=1
    -sEXPORTED_RUNTIME_METHODS=ccall,cwrap
  )
  add_custom_command(TARGET ss_single POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/web ${CMAKE_BINARY_DIR}/single)
else()
  # --- Native test driver ---
  add_executable(ss_test src/main_test.cpp)
  target_link_libraries(ss_test PRIVATE sscore)

  # --- Pure-C++ HTTP server ---
  add_executable(ss_server src/server/main.cpp)
  target_link_libraries(ss_server PRIVATE sscore)
endif()
