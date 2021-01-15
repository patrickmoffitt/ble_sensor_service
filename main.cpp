#include <iostream>
#include <map>
#include <chrono>
#include <thread>
#include <cmath>
#include <signal.h>
#include <string>
#include <functional>
#include "ble_sensor_service.hpp"
#include "ble_bme_680_sqlite3.hpp"

#include <blepp/logging.h>
#include <blepp/blestatemachine.h>
#include <blepp/pretty_printers.h>

#define FALLBACK_TIMEZONE "EST5EDT"

using namespace std;
using namespace chrono;
using namespace BLEPP;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
int main() {
    const char* env_tz = getenv("TZ");
    if(env_tz == nullptr) {
        setenv("TZ", FALLBACK_TIMEZONE, 1);
    }
    log_level = Error;
    BLEGATTStateMachine gatt;

    // Catch the interrupt signal. If the scanner is not
    // cleaned up properly, then it doesn't reset the HCI state.
    signal(SIGINT, BLE_sensor_service::catch_function);
    signal(SIGHUP, BLE_sensor_service::catch_function);

    BLE_sensor_service svc;

    Ble_bme_680_db db("/home/patrick/SQLite3", "ble_bme680.db");
    /*
     * Open the database file and see if the sensor_data
     * table is there. If not, create it.
     */
    if (!db.sensor_table_exists()) {
        db.create_sensor_data_table();
        db.create_sensor_data_table_idx();
    }

    function<void(const PDUReadResponse&)> read_value =
            [&](const PDUReadResponse& r) {
                // (nRF52840 + BME680), on Zephyr, sends signed little-endian int32
                uint32_t value;
                svc.swap_end32(value, r.data, r.length);
                LOG(Info, "read_cb "
                << svc.sensors[svc.sens_idx].label << ": "
                << setprecision(2)
                << value * pow(10, svc.sensors[svc.sens_idx].exponent));
                svc.sensors[svc.sens_idx].value = value;
            };

    function<void(const PDUNotificationOrIndication&)> notify_cb =
            [&](const PDUNotificationOrIndication& n) {
                // (nRF52840 + BME680), on Zephyr, sends signed little-endian int32
                int32_t value = n.data[6] << 24 | n.data[5] << 16 | n.data[4] << 8 | n.data[3];

                LOG(Info, "notify_cb | " << setprecision(4) 
                    << svc.sensors[svc.sens_idx].label << ": "
                    << value * pow(10, svc.sensors[svc.sens_idx].exponent));
            };

    function<void()> found_services_and_characteristics_cb =
            [&]() {
                LOG(Debug, "Finding services and characteristics.");
                for (auto &service: gatt.primary_services) {
                    for (auto &characteristic: service.characteristics) {
                        LOG(Debug, "seeking: " << to_str(characteristic.uuid));
                        auto seek = svc.sensors.find(
                                to_str(characteristic.uuid).c_str());
                        if (seek != svc.sensors.end()) {
                            LOG(Debug, "svc::sensors UUID: " << seek->first);
                            seek->second.handle = characteristic.value_handle;
                        }
                    }
                }
                svc.handles_on = true;
            };

    gatt.cb_disconnected = [&](
            BLEGATTStateMachine::Disconnect d) {
        LOG(Info, "Disconnect for reason: "
                << BLEGATTStateMachine::get_disconnect_string(d));

        this_thread::sleep_for(2s); // Don't reconnect immediately.
        svc.ble_mac_addr = svc.get_mac_by_uuid(TARGET_SERVICE_UUID);
        gatt.connect_blocking_randaddr(svc.ble_mac_addr);
    };

    gatt.setup_standard_scan(found_services_and_characteristics_cb);
    gatt.connect_blocking_randaddr(svc.ble_mac_addr);

    while(1) {
        gatt.setup_standard_scan(found_services_and_characteristics_cb);

        while (not svc.handles_on)
            gatt.read_and_process_next();

        for (const auto &sensor: svc.sensors) {
            auto characteristic = gatt.characteristic_of_handle(
                    sensor.second.handle);
            if (characteristic != nullptr) {
                svc.sens_idx = sensor.first;
                characteristic->cb_read = read_value;
                characteristic->read_request();
                while (sensor.second.value == -1)
                    gatt.read_and_process_next();
            } else {
                LOG(Error, "No characteristic found for handle: "
                        << sensor.second.handle);
            }
        }
        db.sensor_data_insert(svc.sensors[svc.temperature_str].value,
                              svc.sensors[svc.temperature_str].exponent,

                              svc.sensors[svc.humidity_str].value,
                              svc.sensors[svc.humidity_str].exponent,

                              svc.sensors[svc.pressure_str].value,
                              svc.sensors[svc.pressure_str].exponent,

                              svc.sensors[svc.gas_str].value,
                              svc.sensors[svc.gas_str].exponent,

                              svc.sensors[svc.battery_str].value,
                              svc.sensors[svc.battery_str].exponent);

        auto now = system_clock::to_time_t(system_clock::now());
        auto then = system_clock::to_time_t(system_clock::now() - 4min);
        LOG(Info, db.get_sensor_data_range(then, now));

        auto characteristic = gatt.characteristic_of_handle(
                svc.sensors[svc.notify_str].handle);
        if (characteristic != nullptr) {
            uint8_t one[] = {0x01, 0x00}; //  Little endian one.
            characteristic->write_request(one);
            gatt.close();
            LOG(Info, "Acknowledged receipt of sensor data.");
        }

        LOG(Info, "Getting ready to loop.");

        svc.handles_on = false;
        for (auto &sensor: svc.sensors) {
            sensor.second.handle = 0;
            sensor.second.value = -1;
        }

    }

    return 0;
}
#pragma clang diagnostic pop
