#!/bin/bash

pushd .
cd /home/pi/log_serial

current_accessed_log_file=$(sudo lsof -p `cat log_serial.pid` | grep '/.*\.log' -o)
cur_f=$(basename $current_accessed_log_file)

for f in `ls log/*.log`; do
	f=$(basename $f)
	if [ "${f}" != "${cur_f}" ]; then
		dropbox_uploader/dropbox_uploader.sh upload "$f" log/ >&2
		trash "$f"
	fi
done

popd
exit 0
