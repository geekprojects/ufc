cmake_minimum_required(VERSION 3.13)
project(UniversalFlightConnector VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(yamlcpp REQUIRED yaml-cpp)
pkg_check_modules(lua REQUIRED lua)

find_library(SQLITE3_LIBRARY NAMES sqlite3 )

if(UNIX AND NOT APPLE)
    pkg_check_modules(hidapi REQUIRED hidapi-libusb)
    set(OS_LIBS )
elseif(UNIX AND APPLE)
    pkg_check_modules(hidapi REQUIRED hidapi)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(FOUNDATION_LIBRARY Foundation)
    set(OS_LIBS ${IOKIT_LIBRARY} ${FOUNDATION_LIBRARY})
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# Third Party Modules
add_subdirectory(3rdparty/luacpp/Source EXCLUDE_FROM_ALL)

set(DATADIR ${CMAKE_INSTALL_PREFIX}/share/ufc)
SET(FLAGS_COMMON "-Wall -Werror -Wno-vla-cxx-extension -Wno-unused-function -Wno-unknown-warning-option")
SET(CMAKE_CXX_FLAGS_DEBUG "${FLAGS_COMMON} -O0 -g -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
SET(CMAKE_CXX_FLAGS_RELEASE  "${FLAGS_COMMON} -O3")

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/3rdparty/luacpp/Source)
add_definitions(${yamlcpp_CFLAGS} ${hidapi_CFLAGS} ${lua_CFLAGS})

add_library(ufc SHARED
        src/libufc/flightconnector.cpp
        src/libufc/utils/data.cpp
        src/libufc/utils/logger.cpp
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
        include/ufc/usbhiddevice.h
        src/libufc/devices/usbhiddevice.cpp
        src/libufc/datadefs.h
        include/ufc/utils.h
        src/libufc/utils/utils.cpp
        include/ufc/airports.h
        src/libufc/airport.cpp
        include/ufc/geoutils.h
        src/libufc/utils/geoutils.cpp
        src/libufc/navdata/xplane/airports.cpp
        src/libufc/navdata/xplane/navaids.cpp
        src/libufc/datasources/datasource.cpp
        src/libufc/datasources/clock.cpp
        src/libufc/datasources/clock.h
        include/ufc/navdata.h
        src/libufc/datasources/aircraftmapping.cpp
        include/ufc/aircraftmapping.h
        include/ufc/udp.h
        src/libufc/utils/udp.cpp
        src/libufc/lua.h
        src/libufc/lua.cpp
        include/ufc/bitbuffer.h
        src/libufc/utils/bitbuffer.cpp
        src/libufc/devices/usbhidconfig/device.cpp
        src/libufc/devices/usbhidconfig/usbhidconfig.h
        src/libufc/aircraftstate.cpp
        src/libufc/devices/usbhidconfig/lcd.h
        src/libufc/devices/usbhidconfig/manager.cpp
        src/libufc/navdata/xplane/xplanenavdata.cpp
        src/libufc/navdata/xplane/xplanenavdata.h
        src/libufc/navdata/littlenavmap/lnmnavdata.cpp
        src/libufc/navdata/littlenavmap/lnmnavdata.h
        src/libufc/devices/serial/serialmanager.cpp
        src/libufc/devices/serial/serial.h
        src/libufc/devices/serial/lib/list_ports_linux.cpp
        src/libufc/devices/serial/lib/list_ports_osx.cpp
        src/libufc/devices/serial/lib/list_ports_win.cpp
        src/libufc/devices/serial/lib/serial_unix.cpp
        src/libufc/devices/serial/lib/serial_win.cpp
        src/libufc/devices/serial/lib/serial.cpp
        src/libufc/devices/serial/device.cpp
        src/libufc/utils/database.cpp
        src/libufc/devices/serial/serialport.cpp
        src/libufc/devices/serial/serialport.h
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
        ${OS_LIBS}
        ${SQLITE3_LIBRARY}
)

if (UNIX AND APPLE AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(ufc PROPERTIES
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    )
endif()

install(TARGETS ufc DESTINATION lib)

add_executable(ufctool src/ufctool/main.cpp)
target_link_libraries(ufctool
        ufc
)

if (UNIX AND APPLE AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(ufctool PROPERTIES
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    )
endif()

if(UNIX AND APPLE)
    set(XPLM_LDFLAGS ${CMAKE_SOURCE_DIR}/3rdparty/XPlaneSDK/Libraries/Mac/XPLM.framework)
    set(XPLM_CFLAGS -DAPL=1)
elseif (UNIX AND NOT APPLE)
    set(XPLM_CFLAGS -DLIN=1)
endif()

set(XPLANE_INC ${CMAKE_SOURCE_DIR}/3rdparty/XPlaneSDK/CHeaders/XPLM)

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
        ufc
        ${XPLM_LDFLAGS}
)

if (UNIX AND APPLE AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(ufc-plugin PROPERTIES
            XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    )
endif()

set(DATADIR ${CMAKE_INSTALL_PREFIX}/share/ufc)
install(DIRECTORY include/ufc DESTINATION include FILES_MATCHING PATTERN "*.h" )
install(DIRECTORY data/x-plane DESTINATION ${DATADIR})
install(DIRECTORY data/usbhid DESTINATION ${DATADIR})

set(PKGCONFIG_CFLAGS_LIST -I${CMAKE_INSTALL_PREFIX}/include ${yamlcpp_CFLAGS} ${hidapi_CFLAGS})
string(REPLACE ";" " " PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS_LIST}")
set(PKGCONFIG_LDFLAGS_LIST -L${CMAKE_INSTALL_PREFIX}/lib -lufc ${yamlcpp_LDFLAGS} ${hidapi_LDFLAGS})
string(REPLACE ";" " " PKGCONFIG_LDFLAGS "${PKGCONFIG_LDFLAGS_LIST}")

configure_file(libufc.pc.in libufc.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/libufc.pc DESTINATION "lib${LIB_SUFFIX}/pkgconfig")

