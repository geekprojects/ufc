cmake_minimum_required(VERSION 3.13)
project(UniversalFlightConnector VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(yamlcpp REQUIRED yaml-cpp)
pkg_check_modules(lua REQUIRED lua)
pkg_check_modules(nlohmann_json REQUIRED nlohmann_json)

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

set(USE_EMBEDDED_CURL ON)
set(USE_SYSTEM_CURL OFF)
set(CURL_DISABLE_WEBSOCKETS OFF CACHE BOOL "" FORCE)
add_subdirectory(3rdparty/curly.hpp EXCLUDE_FROM_ALL)

set(DATADIR ${CMAKE_INSTALL_PREFIX}/share/ufc)
SET(FLAGS_COMMON "-Wall -Werror -Wno-vla-cxx-extension -Wno-unused-function -Wno-unknown-warning-option")
SET(CMAKE_CXX_FLAGS_DEBUG "${FLAGS_COMMON} -O0 -g -fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
SET(CMAKE_CXX_FLAGS_RELEASE  "${FLAGS_COMMON} -O3")

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/3rdparty/luacpp/Source
        ${CMAKE_SOURCE_DIR}/3rdparty/curly.hpp/headers
)
add_definitions(${yamlcpp_CFLAGS} ${hidapi_CFLAGS} ${lua_CFLAGS} ${nlohmann_json_CFLAGS} ${curl_CFLAGS})

add_library(ufc-utils SHARED
        src/libufc-utils/data.cpp
        src/libufc-utils/logger.cpp
        src/libufc-utils/utils.cpp
        src/libufc-utils/geoutils.cpp
        src/libufc-utils/udp.cpp
        src/libufc-utils/bitbuffer.cpp
        src/libufc-utils/database.cpp
        include/ufc/utils/geoutils.h
        include/ufc/utils/bitbuffer.h
        include/ufc/utils/logger.h
        include/ufc/utils/data.h
        include/ufc/utils/utils.h
        include/ufc/utils/udp.h
        src/libufc-utils/base64.cpp
        src/libufc-utils/base64.h
)

target_compile_definitions(ufc-utils PUBLIC -DDATADIR=${DATADIR})
target_link_libraries(ufc-utils
        luacpp_static
        ${SQLITE3_LIBRARY}
)

add_library(ufc-data SHARED
        src/libufc-data/navdata/airport.cpp
        src/libufc-data/navdata/xplane/xplanenavdata.cpp
        include/ufc/data/xplanenavdata.h
        src/libufc-data/navdata/xplane/airports.cpp
        src/libufc-data/navdata/xplane/navaids.cpp
        src/libufc-data/navdata/littlenavmap/lnmnavdata.cpp
        include/ufc/data/lnmnavdata.h
        src/libufc-data/navdata/littlenavmap/lnmairports.cpp
        src/libufc-data/navdata/littlenavmap/lnmnavaids.cpp
        include/ufc/data/greatcircle.h
        src/libufc-data/greatcircle.cpp
        src/libufc-data/flightplans/routetext.cpp
        include/ufc/data/airports.h
        include/ufc/data/navdata.h
        include/ufc/aircraftmapping.h
        src/libufc-data/navdata/littlenavmap/lnmprocedures.cpp
        src/libufc-data/navdata/littlenavmap/lnmairways.cpp
        include/ufc/data/navaids.h
        include/ufc/data/airways.h
        src/libufc-data/navdata/procedures.cpp
        include/ufc/data/flightplan.h
        include/ufc/data/fpformat.h
        include/ufc/data/routetextformat.h
        src/libufc-data/flightplans/fpformat.cpp
        include/ufc/data/msfsiniformat.h
        include/ufc/data/simbriefjson.h
        include/ufc/data/xplanefmsformat.h
        src/libufc-data/flightplans/msfsiniformat.cpp
        src/libufc-data/flightplans/simbriefjson.cpp
)
target_compile_definitions(ufc-data PUBLIC -DDATADIR=${DATADIR})
target_link_libraries(ufc-data
        ufc-utils
        ${SQLITE3_LIBRARY}
        curl
)

add_library(ufc SHARED
        src/libufc-connector/flightconnector.cpp
        src/libufc-connector/datasources/simulator.cpp
        src/libufc-connector/datasources/simulator.h
        src/libufc-connector/datasources/xplane/xplane.cpp
        src/libufc-connector/datasources/xplane/xplane.h
        src/libufc-connector/datasources/xplane/xplaneudpclient.cpp
        src/libufc-connector/datasources/xplane/xplaneclient.h
        src/libufc-connector/devices/consoleoutput.cpp
        src/libufc-connector/devices/consoleoutput.h
        src/libufc-connector/devices/device.cpp
        include/ufc/aircraftstate.h
        include/ufc/datasource.h
        include/ufc/devices/device.h
        include/ufc/flightconnector.h
        include/ufc/devices/usbhiddevice.h
        src/libufc-connector/devices/usbhiddevice.cpp
        src/libufc-connector/datasources/datasource.cpp
        src/libufc-connector/datasources/clock.cpp
        src/libufc-connector/datasources/clock.h
        src/libufc-connector/datasources/aircraftmapping.cpp
        src/libufc-connector/lua.h
        src/libufc-connector/lua.cpp
        src/libufc-connector/devices/usbhidconfig/device.cpp
        src/libufc-connector/devices/usbhidconfig/usbhidconfig.h
        src/libufc-connector/aircraftstate.cpp
        src/libufc-connector/devices/usbhidconfig/lcd.h
        src/libufc-connector/devices/usbhidconfig/manager.cpp
        src/libufc-connector/devices/serial/serialmanager.cpp
        src/libufc-connector/devices/serial/serial.h
        src/libufc-connector/devices/serial/lib/list_ports_linux.cpp
        src/libufc-connector/devices/serial/lib/list_ports_osx.cpp
        src/libufc-connector/devices/serial/lib/list_ports_win.cpp
        src/libufc-connector/devices/serial/lib/serial_unix.cpp
        src/libufc-connector/devices/serial/lib/serial_win.cpp
        src/libufc-connector/devices/serial/lib/serial.cpp
        src/libufc-connector/devices/serial/device.cpp
        src/libufc-connector/devices/serial/serialport.cpp
        src/libufc-connector/devices/serial/serialport.h
        src/libufc-connector/datasources/xplane/xplaneudpclient.h
        src/libufc-connector/datasources/xplane/xplanewsclient.cpp
        src/libufc-connector/datasources/xplane/xplanewsclient.h
        src/libufc-connector/datasources/xplane/xplaneclient.cpp
)

if(UNIX AND APPLE)
target_link_options(ufc PUBLIC -all_load)
endif()

target_compile_definitions(ufc PUBLIC -DDATADIR=${DATADIR})
target_link_libraries(ufc
        ufc-utils
        luacpp_static
        ${yamlcpp_LDFLAGS}
        ${hidapi_LDFLAGS}
        ${lua_LDFLAGS}
        ${OS_LIBS}
        ${SQLITE3_LIBRARY}
        curly.hpp::curly.hpp
)

if (UNIX AND APPLE AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(ufc-utils PROPERTIES
            XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    )
    set_target_properties(ufc-data PROPERTIES
            XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    )
    set_target_properties(ufc PROPERTIES
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
    )
endif()

install(TARGETS ufc ufc ufc-utils ufc-data DESTINATION lib)

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

add_subdirectory(tests)

set(DATADIR ${CMAKE_INSTALL_PREFIX}/share/ufc)
install(DIRECTORY include/ufc DESTINATION include FILES_MATCHING PATTERN "*.h" )
install(DIRECTORY data/x-plane DESTINATION ${DATADIR})
install(DIRECTORY data/usbhid DESTINATION ${DATADIR})

set(PKGCONFIG_CFLAGS_LIST -I${CMAKE_INSTALL_PREFIX}/include ${yamlcpp_CFLAGS} ${hidapi_CFLAGS})
string(REPLACE ";" " " PKGCONFIG_CFLAGS "${PKGCONFIG_CFLAGS_LIST}")
set(PKGCONFIG_LDFLAGS_LIST -L${CMAKE_INSTALL_PREFIX}/lib -lufc-utils -lufc ${yamlcpp_LDFLAGS} ${hidapi_LDFLAGS})
string(REPLACE ";" " " PKGCONFIG_LDFLAGS "${PKGCONFIG_LDFLAGS_LIST}")

configure_file(libufc.pc.in libufc.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/libufc.pc DESTINATION "lib${LIB_SUFFIX}/pkgconfig")
