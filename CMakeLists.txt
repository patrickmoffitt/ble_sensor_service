#
#  BLE Sensor Service
#
#  Copyright (C) 2020 Patrick K. Moffitt
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

cmake_minimum_required(VERSION 3.4)
project(ble_sensor_service)
set(BLE_PROJECT_NAME ble++)

set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -pthread)
set(CMAKE_CXX_STANDARD 14)

set(HEADERS
        blepp/bledevice.h
        blepp/logging.h
        blepp/float.h
        blepp/uuid.h
        blepp/pretty_printers.h
        blepp/gap.h
        blepp/lescan.h
        blepp/xtoa.h
        blepp/att.h
        blepp/blestatemachine.h
        blepp/att_pdu.h
        ble_sensor_service.hpp
        ble_bme_680_sqlite3.hpp
        jsonxx/jsonxx.h)

set(SRC
        src/att_pdu.cc
        src/float.cc
        src/logging.cc
        src/uuid.cc
        src/blestatemachine.cc
        src/bledevice.cc
        src/pretty_printers.cc
        src/att.cc
        src/lescan.cc
        ${HEADERS}
        ble_sensor_service.cpp
        ble_bme_680_sqlite3.cpp
        jsonxx/jsonxx.cc )

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(Bluez REQUIRED)

find_package(Boost 1.71.0 COMPONENTS filesystem REQUIRED)

include_directories(${PROJECT_SOURCE_DIR} ${BLUEZ_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
add_library(${BLE_PROJECT_NAME} SHARED ${SRC})
set_target_properties(${BLE_PROJECT_NAME} PROPERTIES SOVERSION 5)


add_executable(${PROJECT_NAME} ${SOURCE_FILES} main.cpp)
target_link_libraries(${PROJECT_NAME} ${BLE_PROJECT_NAME} ${BLUEZ_LIBRARIES} ${Boost_LIBRARIES})

target_link_options(${PROJECT_NAME} PRIVATE LINKER:-lsqlite3)


install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
install(DIRECTORY blepp DESTINATION include)

# On linux the service must run with elevated capabilities to listen to Bluetooth.
#add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD
#        COMMAND bash "-c" "pkexec --user root setcap 'cap_net_raw,cap_net_admin+eip' ${PROJECT_BINARY_DIR}/${PROJECT_NAME}"
#        VERBATIM
#        USES_TERMINAL)
