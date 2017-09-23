cmake_minimum_required(VERSION 2.8)
project(BasicTV CXX)

find_package(PkgConfig)

file(GLOB_RECURSE BasicTV_SRC
  "src/*.h"
  "src/*.cpp")

add_executable(basictv ${BasicTV_SRC})

# TODO: be more flexible in requirements

pkg_check_modules(LIBS REQUIRED portaudio-2.0 opus opusfile SDL2_net sdl2 libcurl zlib openssl libzstd ogg)

target_link_libraries(basictv ${LIBS_LIBRARIES})
target_include_directories(basictv PUBLIC ${LIBS_INCLUDE_DIRS})
target_compile_options(basictv PUBLIC ${LIBS_CFLAGS_OTHER})


target_link_libraries(basictv ${LIBS})
