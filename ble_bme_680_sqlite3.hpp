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

#ifndef SQLITE3_BLE_BME_680_SQLITE3_HPP
#define SQLITE3_BLE_BME_680_SQLITE3_HPP

#include <iostream>
#include <cstring>
#include <chrono>
#include <ctime>
#include <cmath>
#include <sqlite3.h>
#include "boost/filesystem.hpp"
#include "jsonxx/jsonxx.h"

#define EXIT_DB_DIRECTORY_ERROR                             101
#define EXIT_DB_HANDLE_ERROR                                102
#define EXIT_DB_PREP_STMT_SENSOR_DATA_TBL_COUNT             103
#define EXIT_DB_EXEC_STMT_SENSOR_TABLE_EXISTS               104

#define EXIT_DB_PREP_STMT_CREATE_SENSOR_TABLE               105
#define EXIT_DB_EXEC_STMT_CREATE_SENSOR_TABLE               106

#define EXIT_DB_PREP_STMT_CREATE_SENSOR_TABLE_IDX           107
#define EXIT_DB_EXEC_STMT_CREATE_SENSOR_TABLE_IDX           108

#define EXIT_DB_PREP_STMT_SENSOR_DATA_INSERT                109
#define EXIT_DB_EXEC_STMT_SENSOR_DATA_INSERT                110

#define EXIT_DB_PREP_STMT_SENSOR_DATA_BIND_E                111
#define EXIT_DB_PREP_STMT_SENSOR_DATA_BIND_T                112
#define EXIT_DB_PREP_STMT_SENSOR_DATA_BIND_H                113
#define EXIT_DB_PREP_STMT_SENSOR_DATA_BIND_P                114
#define EXIT_DB_PREP_STMT_SENSOR_DATA_BIND_G                115
#define EXIT_DB_PREP_STMT_SENSOR_DATA_BIND_B                116

#define EXIT_DB_PREP_STMT_GET_SENSOR_DATA_RANGE             117
#define EXIT_DB_EXEC_STMT_GET_SENSOR_DATA_RANGE             118

#define EXIT_DB_PREP_STMT_GET_SENSOR_DATA_RANGE_BIND_B      119
#define EXIT_DB_PREP_STMT_GET_SENSOR_DATA_RANGE_BIND_E      120

using namespace std;
using namespace chrono;
namespace std {
    namespace filesystem = boost::filesystem;
}
namespace fs = std::filesystem;
using namespace jsonxx;

/*
 * @brief Factory for sensor data operations.
 */
class Ble_bme_680_db {
public:

    sqlite3 *db_h{nullptr};

    Ble_bme_680_db(const char *sqlite3_db_dir,
                   const char *sqlite3_db_file);
    ~Ble_bme_680_db();

    const char *sensor_table_exists_stmt =
            R"(SELECT COUNT(*)
            FROM sqlite_master
            WHERE type='table'
            AND name='sensor_data')";

    const char *create_sensor_data_table_stmt =
            R"(CREATE TABLE "main"."sensor_data" (
                "epoch" integer NOT NULL,
                "temperature_c" real,
                "humidity_rh" real,
                "pressure_kpa" real,
                "gas_ohms" integer,
                "battery_volts" real,
                UNIQUE (epoch COLLATE BINARY DESC) ))";

    const char *create_sensor_data_table_idx_stmt =
            R"(CREATE INDEX "main"."epoch_idx"
                ON "sensor_data"
                ("epoch" COLLATE BINARY DESC))";

    const char *sensor_data_insert_stmt =
            R"(INSERT OR REPLACE
                INTO 'sensor_data'
                ('epoch', 'temperature_c',
                'humidity_rh', 'pressure_kpa',
                'gas_ohms', 'battery_volts')
                VALUES (?, ?, ?, ?, ?, ?))";

    const char *get_sensor_data_range_stmt =
            R"(SELECT * FROM sensor_data
                WHERE epoch <= ?
                AND epoch >= ?
                ORDER BY epoch ASC)";

    bool sensor_table_exists() const;
    void create_sensor_data_table() const;
    void create_sensor_data_table_idx() const;
    void sensor_data_insert(int32_t t, int8_t t_exp,
                            int32_t h, int8_t h_exp,
                            int32_t p, int8_t p_exp,
                            int32_t g, int8_t g_exp,
                            int32_t b, int8_t b_exp) const;
    string get_sensor_data_range(int32_t epoch1, int32_t epoch2) const;
    void on_error_exit(int error, int condition, int exit_code) const;
};

#endif //SQLITE3_BLE_BME_680_SQLITE3_HPP
