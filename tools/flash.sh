#!/bin/bash

ESP32_PATH="${HOME}/Workspace/Arduino/hardware/espressif/esp32"
BOOT_LOADER="${ESP32_PATH}/tools/sdk/bin/bootloader_dio_80m.bin"
BOOT_APP0="${ESP32_PATH}/tools/partitions/boot_app0.bin"

MAIN_INO="src/tmp/Main.ino.bin"
MAIN_PARTITIONS_INO="src/tmp/Main.ino.partitions.bin"


function flash {
    port=$1
    [[ "${port}" == "" ]] && port='/dev/ttyUSB0'
    echo ">>> Flashing.. on ${port}"

    esptool.py --chip esp32 --port ${port} --baud 921600  \
        --before default_reset --after hard_reset  \
        write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect  \
        0x1000  "${BOOT_LOADER}"  \
        0x8000  "${MAIN_PARTITIONS_INO}"  \
        0xe000  "${BOOT_APP0}"  \
        0x10000 "${MAIN_INO}"
}


ports=`seq 0 2`
for p in ${ports}; do
    flash /dev/ttyUSB${p}
done
