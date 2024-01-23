cmake_minimum_required(VERSION 3.27)
project(xpfd)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(sdl2 sdl2)
pkg_check_modules(libgeek REQUIRED libgeek)
pkg_check_modules(yamlcpp REQUIRED yaml-cpp)
pkg_check_modules(glm REQUIRED glm)

SET(FLAGS_COMMON "-Wall -Werror")
SET(CMAKE_CXX_FLAGS_DEBUG "${FLAGS_COMMON} -O0 -g -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
SET(CMAKE_CXX_FLAGS_RELEASE  "${FLAGS_COMMON} -O3")

include_directories(${CMAKE_SOURCE_DIR}/src)
add_definitions(-I/usr/local/include ${sdl2_CFLAGS} ${libgeek_CFLAGS} ${yamlcpp_CFLAGS} ${glm_CFLAGS})

add_executable(xpfd src/main.cpp src/xplane.cpp
        src/xplane.h
        src/display.cpp
        src/display.h
        src/gfxutils.cpp
        src/gfxutils.h
        src/state.cpp
        src/state.h
        src/widgets/adi.cpp
        src/widgets/adi.h
        src/widgets/speedindicator.cpp
        src/widgets/speedindicator.h
        src/widgets/altitudeindicator.cpp
        src/widgets/altitudeindicator.h
        src/widgets/headingindicator.cpp
        src/widgets/headingindicator.h
)

target_link_libraries(xpfd
        -Wl,-rpath -Wl,/usr/local/lib
        -L/usr/local/lib
        -lbrick
        ${libgeek_LDFLAGS}
        ${sdl2_LDFLAGS}
        ${yamlcpp_LDFLAGS}
        ${glm_LDFLAGS}
)
