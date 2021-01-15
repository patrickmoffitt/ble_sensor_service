How to Capture Bluetooth Low Energy Broadcasts in a SQLite Database
-------------------------------------------------------------------
This project captures the broadcasts from my 
[Bluetooth Low Energy Environmental Sensor](https://github.com/patrickmoffitt/zephyr_ble_sensor) and stores them in a 
SQLite database. I've also provided
a [CGI](https://github.com/patrickmoffitt/ble_sensor_cgi) so you can RESTfully query the data in JSON and inspect it
with [interactive data graphics](https://github.com/patrickmoffitt/ble_sensor_charts) powered by
[Plotly](https://plotly.com/javascript/).

Building and Running
--------------------
This is a CMake project that requires Bluez and Boost. I built it on Ubuntu 20.04 for the Raspberry Pi model 4b. I 
found that both the Pi's internal Bluetooth radio and an external dongle I tried produced very limited reception. The 
best I could get from the Pi was reception from the next room. By contrast when I used a typical Dell laptop from 2015 
reception worked from anywhere in the house and even across the street. 

On linux the service must run with elevated capabilities to listen to Bluetooth. You can grant these capabilities like 
so:
```bash
sudo setcap 'cap_net_raw,cap_net_admin+eip' build_dir_path/project_binary_name
```

Installing as a System Service (daemon)
---------------------------------------
-1. Make a directory named `SQLite3` in your home directory.

-2. Copy the project binary and the contents of the `run` directory from this project into it.

-3. Edit `ble_sensor.service` to reflect your username, group, and the path to your home directory. Hint: read
the comments.

-4. As root, copy `ble_sensor.service` to `/etc/systemd/system` and enable the
service:
```bash
    sudo systemctl enable ble_sensor.service
    Created symlink /etc/systemd/system/multi-user.target.wants/ble_sensor.service → /etc/systemd/system/ble_sensor.service.
```

-5. Start the new service and check it's status.
```bash
    sudo systemctl start ble_sensor
    sudo systemctl status ble_sensor
    ● ble_sensor.service - Bluetooth Low Energy SQLite3 Service
         Loaded: loaded (/etc/systemd/system/ble_sensor.service; enabled; vendor preset: enabled)
         Active: active (running) since Wed 2021-01-13 16:58:33 EST; 1 day 16h ago
       Main PID: 107224 (ble_sensor_serv)
          Tasks: 1 (limit: 4200)
         CGroup: /system.slice/ble_sensor.service
                 └─107224 /home/patrick/SQLite3/ble_sensor_service
    
    Jan 13 16:58:33 raspi4 systemd[1]: Started Bluetooth Low Energy SQLite3 Service.

```

Credits
-------
- Thank you to [edrosten](https://github.com/edrosten/) for his 
  [Modern clean C++ Bluetooth Low Energy on Linux without the Bluez DBUS API](https://github.com/edrosten/libblepp).
- Thank you to [hjiang](https://github.com/hjiang) for the [jsonxx](https://github.com/hjiang/jsonxx) project. I tested
  several C++ JSON libraries including [jsoncpp](https://github.com/open-source-parsers/jsoncpp) before deciding to use
  jsonxx for its simplicity and speed.  

Bugs, Issues, and Pull Requests
------------------------------
If you find a bug please create an issue. If you'd like to contribute please send a pull request.

References
----------
The following were helpful references in the development of this project.

- [Apache Tutorial: Dynamic Content with CGI](http://httpd.apache.org/docs/current/howto/cgi.html)
- [Common Gateway Interface](https://en.wikipedia.org/wiki/Common_Gateway_Interface)
