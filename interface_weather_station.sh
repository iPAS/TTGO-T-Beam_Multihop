#!/bin/bash

# print off|1000 sys print_off | disable print message
# get nid  |#0   route get_nid | get node ID and network ID
# get rtc  |1000 rtc get 1     | get RTC
# set rtc  |1000 rtc set #d #t | update RTC with host RTC

# print off        |1000 sys print_off  |disable print message
# on 3.3V          |$ pwrsw on 330      |on sensor power 3.3V
# T/H              |1000 i2c get 1099   |read T/H intf
# wind read        |$ wind get 0        |get wind speed and direction (update every 10 sec.)
# rain read        |$ rain get 0        |get rain volumn(CC)
# rain24Hr read    |$ landsld get 0     |get rain volumn24Hr window
# GSM Signal       |$ uc20 get          |get GSM signal
# Battery          |$ charger get       |get Vbatt
# Charging Current |$ atod get 22       |get Charging Current
# Bus Voltage      |$ atod get 20       |get Bus Voltage

pyserial-miniterm /dev/ttyUSB0 38400 --parity E --eol CRLF
