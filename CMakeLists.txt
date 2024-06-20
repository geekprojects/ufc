cmake_minimum_required(VERSION 3.13)
project(UniversalFlightConnector VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(yamlcpp REQUIRED yaml-cpp)

if(UNIX AND NOT APPLE)
pkg_check_modules(hidapi REQUIRED hidapi-libusb)
elseif(UNIX AND APPLE)
pkg_check_modules(hidapi REQUIRED hidapi)
else()
message(FATAL_ERROR "Unsupported platform")
endif()

pkg_check_modules(libgeek libgeek)
pkg_check_modules(glm glm)
pkg_check_modules(sdl2 sdl2)

SET(FLAGS_COMMON "-Wall -Werror")
SET(CMAKE_CXX_FLAGS_DEBUG "${FLAGS_COMMON} -O0 -g -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
SET(CMAKE_CXX_FLAGS_RELEASE  "${FLAGS_COMMON} -O3")

set(DATADIR ${CMAKE_INSTALL_PREFIX}/share/ufc)


include_directories(${CMAKE_SOURCE_DIR}/include)
#add_definitions(-I/usr/local/include ${sdl2_CFLAGS} ${libgeek_CFLAGS} ${yamlcpp_CFLAGS} ${glm_CFLAGS} ${hidapi_CFLAGS})
add_definitions(-I/usr/local/include ${yamlcpp_CFLAGS} ${hidapi_CFLAGS})

add_library(ufc
        src/libufc/aircraftstate.cpp
        src/libufc/flightconnector.cpp
        src/libufc/data.cpp
        src/libufc/logger.cpp
        src/libufc/datasources/simulator.cpp
        src/libufc/datasources/simulator.h
        src/libufc/datasources/xplane/xplane.cpp
        src/libufc/datasources/xplane/xplane.h
        src/libufc/datasources/xplane/xplaneclient.cpp
        src/libufc/datasources/xplane/xplaneclient.h
        src/libufc/devices/consoleoutput.cpp
        src/libufc/devices/consoleoutput.h
        src/libufc/devices/device.cpp
        include/ufc/aircraftstate.h
        include/ufc/datasource.h
        include/ufc/device.h
        include/ufc/flightconnector.h
        include/ufc/logger.h
        include/ufc/data.h
        src/libufc/devices/arduinotest.cpp
        src/libufc/devices/arduinotest.h
        include/ufc/usbhiddevice.h
        src/libufc/devices/usbhiddevice.cpp
        src/libufc/devices/winwingfcu.cpp
        src/libufc/devices/winwingfcu.h
        src/libufc/datasources/xplane/datadefs.h
)

if(UNIX AND APPLE)
target_link_options(ufc PUBLIC -all_load)
endif()

target_compile_definitions(ufc PUBLIC -DDATADIR=${DATADIR})
target_link_libraries(ufc
        ${yamlcpp_LDFLAGS}
        ${hidapi_LDFLAGS}
)

if (${libgeek_FOUND})
add_executable(xpfd
        src/pfd/main.cpp
        src/pfd/display.cpp
        src/pfd/display.h
        src/pfd/gfxutils.cpp
        src/pfd/gfxutils.h
        src/pfd/widgets/adi.cpp
        src/pfd/widgets/adi.h
        src/pfd/widgets/speedindicator.cpp
        src/pfd/widgets/speedindicator.h
        src/pfd/widgets/altitudeindicator.cpp
        src/pfd/widgets/altitudeindicator.h
        src/pfd/widgets/headingindicator.cpp
        src/pfd/widgets/headingindicator.h
        src/pfd/widgets/widget.h
)

target_include_directories(xpfd PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_compile_options(xpfd PUBLIC ${libgeek_CFLAGS} ${sdl2_CFLAGS})

target_link_libraries(xpfd
        -Wl,-rpath -Wl,/usr/local/lib
        -L/usr/local/lib
        -lbrick
        ufc
        ${libgeek_LDFLAGS}
        ${sdl2_LDFLAGS}
        ${yamlcpp_LDFLAGS}
        ${glm_LDFLAGS}
        ${hidapi_LDFLAGS}
)
endif()

add_executable(ufctool src/ufctool/main.cpp)
target_link_libraries(ufctool
        -Wl,-rpath -Wl,/usr/local/lib
        -L/usr/local/lib
        ufc
)

install(DIRECTORY include/ufc DESTINATION include FILES_MATCHING PATTERN "*.h" )

