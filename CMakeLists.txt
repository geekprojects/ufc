cmake_minimum_required(VERSION 3.13)
project(UniversalFlightConnector VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(yamlcpp REQUIRED yaml-cpp)
pkg_check_modules(lua REQUIRED lua)

if(UNIX AND NOT APPLE)
pkg_check_modules(hidapi REQUIRED hidapi-libusb)
elseif(UNIX AND APPLE)
pkg_check_modules(hidapi REQUIRED hidapi)
else()
message(FATAL_ERROR "Unsupported platform")
endif()

# Optional dependencies, used for the PFD display
pkg_check_modules(cairomm cairomm-1.16)
pkg_check_modules(pangomm pangomm-2.48)
pkg_check_modules(freetype2 REQUIRED freetype2)
pkg_check_modules(glm glm)
pkg_check_modules(sdl2 sdl2)

# Third Party Modules
add_subdirectory(luacpp/Source EXCLUDE_FROM_ALL)

set(DATADIR ${CMAKE_INSTALL_PREFIX}/share/ufc)
SET(FLAGS_COMMON "-Wall -Werror")
SET(CMAKE_CXX_FLAGS_DEBUG "${FLAGS_COMMON} -O0 -g -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
SET(CMAKE_CXX_FLAGS_RELEASE  "${FLAGS_COMMON} -O3")

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/luacpp/Source)
add_definitions(-I/usr/local/include ${yamlcpp_CFLAGS} ${hidapi_CFLAGS} ${lua_CFLAGS})

add_library(ufc SHARED
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
        include/ufc/utils.h
        src/libufc/utils.cpp
        include/ufc/airports.h
        src/libufc/airport.cpp
        include/ufc/geoutils.h
        src/libufc/geoutils.cpp
        src/libufc/datasources/xplane/navdata/airports.cpp
        src/libufc/datasources/xplane/navdata/navdata.cpp
        src/libufc/devices/customradiohid.cpp
        src/libufc/devices/customradiohid.h
        src/libufc/datasources/datasource.cpp
        src/libufc/datasources/clock.cpp
        src/libufc/datasources/clock.h
        include/ufc/navdata.h
        src/libufc/datasources/xplane/xpmapping.cpp
        src/libufc/datasources/xplane/xpmapping.h
)

if(UNIX AND APPLE)
target_link_options(ufc PUBLIC -all_load)
endif()

target_compile_definitions(ufc PUBLIC -DDATADIR=${DATADIR})
target_link_libraries(ufc
        luacpp_static
        ${yamlcpp_LDFLAGS}
        ${hidapi_LDFLAGS}
        ${lua_LDFLAGS}
)

install(TARGETS ufc DESTINATION lib)

add_executable(ufctool src/ufctool/main.cpp)
target_link_libraries(ufctool
        -Wl,-rpath -Wl,/usr/local/lib
        -L/usr/local/lib
        ufc
)

if (DEFINED ENV{XPLANE_SDK})
    message("Building XPlane plugin: $ENV{XPLANE_SDK}")

    if(UNIX AND APPLE)
        set(XPLM_LDFLAGS $ENV{XPLANE_SDK}/Libraries/Mac/XPLM.framework)
        set(XPLM_CFLAGS -DAPL=1)
    elseif (UNIX AND NOT APPLE)
        # TODO!
    endif()

    set(XPLANE_INC $ENV{XPLANE_SDK}/CHeaders/XPLM)

    message("XPLM_LDFLAGS: ${XPLM_LDFLAGS}")
    message("XPLANE_INC: ${XPLANE_INC}")

    add_library(ufc-plugin SHARED
            src/ufc-plugin/ufcplugin.cpp
            src/ufc-plugin/ufcplugin.h
            src/ufc-plugin/xpplugindatasource.cpp
            src/ufc-plugin/xpplugindatasource.h
    )

    target_compile_definitions(ufc-plugin PUBLIC ${XPLM_CFLAGS})
    target_include_directories(ufc-plugin PUBLIC ${XPLANE_INC})
    target_link_libraries(ufc-plugin
            -Wl,-rpath -Wl,/usr/local/lib
            -L/usr/local/lib
            ufc
            ${XPLM_LDFLAGS}
    )
endif()

if (${cairomm_FOUND} AND ${pangomm_FOUND})
add_executable(xpfd
        src/pfd/main.cpp
        src/pfd/display.cpp
        src/pfd/display.h
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
target_compile_options(xpfd PUBLIC ${cairomm_CFLAGS} ${pangomm_CFLAGS} ${sdl2_CFLAGS})

target_link_libraries(xpfd
        -Wl,-rpath -Wl,/usr/local/lib
        -L/usr/local/lib
        ufc
        ${cairomm_LDFLAGS}
        ${pangomm_LDFLAGS}
        ${sdl2_LDFLAGS}
        ${yamlcpp_LDFLAGS}
        ${glm_LDFLAGS}
        ${hidapi_LDFLAGS}
        ${freetype2_LDFLAGS}
)
endif()


install(DIRECTORY include/ufc DESTINATION include FILES_MATCHING PATTERN "*.h" )

set(PKGCONFIG_CFLAGS_LIST -I${CMAKE_INSTALL_PREFIX}/include ${yamlcpp_CFLAGS} ${hidapi_CFLAGS})
string(REPLACE ";" " " PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS_LIST}")
set(PKGCONFIG_LDFLAGS_LIST -L${CMAKE_INSTALL_PREFIX}/lib -lufc ${yamlcpp_LDFLAGS} ${hidapi_LDFLAGS})
string(REPLACE ";" " " PKGCONFIG_LDFLAGS "${PKGCONFIG_LDFLAGS_LIST}")

configure_file(libufc.pc.in libufc.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/libufc.pc DESTINATION "lib${LIB_SUFFIX}/pkgconfig")

