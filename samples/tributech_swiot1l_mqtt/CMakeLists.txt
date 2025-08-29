# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tributech_swiot1l_mqtt)
zephyr_include_directories(include)
zephyr_include_directories(include/tt_sdk)
set(TT_SDK_LIB_PATH ${ZEPHYR_BASE}/lib/tt_sdk)
target_link_libraries(app PRIVATE ${TT_SDK_LIB_PATH}/libttsdk.a)
target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/plugins/zephyr/zephyr_flashfs.c)
target_sources(app PRIVATE src/plugins/zephyr/zephyr_heap.c)
target_sources(app PRIVATE src/plugins/zephyr/zephyr_logging.c)
target_sources(app PRIVATE src/plugins/zephyr/zephyr_rtc.c)
target_sources(app PRIVATE src/plugins/zephyr/zephyr_rtos.c)
target_sources(app PRIVATE src/plugins/zephyr/zephyr_socket.c)
target_sources(app PRIVATE src/plugins/zephyr/zephyr_crypt.c)

