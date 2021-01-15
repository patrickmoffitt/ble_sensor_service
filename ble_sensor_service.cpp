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

#include "ble_sensor_service.hpp"

/*
 * @brief Case-insensitive string comparison
 *
 * @return true when the strings match.
 */
bool BLE_sensor_service::istr_equals(const string &a, const string &b) {
    return std::equal(a.begin(), a.end(), b.begin(),
                      [](char a, char b) {
                          return tolower(a) == tolower(b);
                      });
}


BLE_sensor_service::BLE_sensor_service() {

    bt_string_to_uuid(&temperature_uuid, temperature_str.c_str());
    bt_string_to_uuid(&humidity_uuid, humidity_str.c_str());
    bt_string_to_uuid(&pressure_uuid, pressure_str.c_str());
    bt_string_to_uuid(&gas_uuid, gas_str.c_str());
    bt_string_to_uuid(&battery_uuid, battery_str.c_str());
    bt_string_to_uuid(&notify_uuid, notify_str.c_str());

    sensors = {
            {temperature_str, {temperature_uuid, "Temperature", -2}},
            {humidity_str, {humidity_uuid, "Humidity", -2}},
            {pressure_str, {pressure_uuid, "Pressure", -4}},
            {gas_str, {gas_uuid, "Gas Resistance", 0}},
            {battery_str, {battery_uuid, "Battery Volts", -3}},
            {notify_str, {notify_uuid, "Notify"}}
    };

    ble_mac_addr = get_mac_by_uuid(TARGET_SERVICE_UUID);
    sens_idx = "";
    handles_on = false;



}

BLE_sensor_service::~BLE_sensor_service() {
    sensors.clear();
    sens_idx.clear();
    ble_mac_addr.clear();
}

void BLE_sensor_service::catch_function(int sig) {
    switch (sig) {
        case SIGINT:
            LOG(Error, endl << "Interrupted!" << endl);
            break;
        case SIGHUP:
            LOG(Error, endl << "Time to Hangup." << endl);
            break;
    }
    exit(sig);
}

/*
 * @brief Scan the system's default HCI device for a given UUID.
 *
 * @param service_uuid the UUIS to search for.
 *
 * @return The MAC address corresponding to the service.
 */
string BLE_sensor_service::get_mac_by_uuid(string service_uuid) {
    LOG(Info, "Seeking a MAC address for service: " << service_uuid);
    string ble_mac_addr{};
    HCIScanner::ScanType type = HCIScanner::ScanType::Active;
    HCIScanner::FilterDuplicates filter = HCIScanner::FilterDuplicates::Software;

    int loops{0};
    while (ble_mac_addr.empty()) {
        if (loops > 0) {
            LOG(Debug, "Service UUID not found. Sleeping.");
            this_thread::sleep_for(500ms);
        }
        HCIScanner scanner(true, filter, type);

        //Check to see if there's anything to read from the HCI
        //and wait if there's not.
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 300000;

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(scanner.get_fd(), &fds);
        int err = select(scanner.get_fd() + 1, &fds, NULL, NULL, &timeout);

        //Interrupted, so quit and clean up properly.
        if (err < 0 && errno == EINTR) {
            scanner.stop();
            return ble_mac_addr;
        }

        if (FD_ISSET(scanner.get_fd(), &fds)) {
            //Only read id there's something to read
            vector<AdvertisingResponse> ads = scanner.get_advertisements();
            for (const auto &ad: ads) {
                for (const auto &uuid: ad.UUIDs) {
                    if (istr_equals(to_str(uuid), service_uuid)) {
                        scanner.stop();
                        LOG(Debug, "Found device: " << ad.address << " "
                                                    << "  Service: "
                                                    << to_str(uuid));
                        ble_mac_addr = ad.address;
                    }
                }
            }
        }
        scanner.stop();
        loops++;
    }
    return ble_mac_addr;
}

void BLE_sensor_service::swap_end32(uint32_t &output,
                                    const uint8_t *data,
                                    size_t data_length) {
    size_t b4, b3, b2, b1;
    b4 = data_length -1;
    b3 = data_length -2;
    b2 = data_length -3;
    b1 = data_length -4;
    if (data_length >= 5)
        output = data[b4] << 24 | data[b3] << 16 | data[b2] << 8 | data[b1];
}

