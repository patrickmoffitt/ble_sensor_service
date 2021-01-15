/*
 *  BLE Sensor Service
 *
 *  Copyright (C) 2020 Patrick K. Moffitt
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef BLE_SENSOR_SERVICE_BLE_SENSOR_SERVICE_HPP
#define BLE_SENSOR_SERVICE_BLE_SENSOR_SERVICE_HPP

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <map>
#include <signal.h>
#include <cstdlib>
#include <cmath>
#include <cctype>
#include <unistd.h>
#include <cerrno>
#include <vector>

#include <blepp/pretty_printers.h>
#include <blepp/logging.h>
#include <blepp/lescan.h>
#include <blepp/uuid.h>

#define TARGET_SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"

using namespace std;
using namespace BLEPP;

class BLE_sensor_service {
private:
    bt_uuid_t temperature_uuid;
    bt_uuid_t humidity_uuid;
    bt_uuid_t pressure_uuid;
    bt_uuid_t gas_uuid;
    bt_uuid_t battery_uuid;
    bt_uuid_t notify_uuid;

    typedef struct {
        bt_uuid_t uuid;
        string label = {};
        int8_t exponent = 0;
        uint16_t handle = 0;
        int32_t value = -1;
    } sensor;

    static bool istr_equals(const string &a, const string &b);

public:
    const string temperature_str{"2a6e"};
    const string humidity_str{"2a6f"};
    const string pressure_str{"2a6d"};
    const string gas_str{"12345678-9012-3456-7890-123456abcdef"};
    const string battery_str{"2a19"};
    const string notify_str{"12345678-9012-3456-7890-123456abcdee"};

    BLE_sensor_service();
    ~BLE_sensor_service();

    map<string, sensor> sensors;
    string sens_idx;
    bool handles_on;

    string ble_mac_addr;

    static void catch_function(int sig);
    string get_mac_by_uuid(string service_uuid);
    void swap_end32(uint32_t &output, const uint8_t* data, size_t data_length);

};

#endif //BLE_SENSOR_SERVICE_BLE_SENSOR_SERVICE_HPP
