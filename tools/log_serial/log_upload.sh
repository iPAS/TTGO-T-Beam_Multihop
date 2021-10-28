#!/bin/bash

pushd .
cd /home/pi/log_serial
conf_f=/home/pi/.dropbox_uploader
#echo $cur_f
log_dir=log
dropbox_dir=log/

for f in `ls ${log_dir}/*.log`; do
	f=$(basename $f)
	echo "File: $f"

	current_accessed_log_file=$(sudo lsof -p `cat log_serial.pid` | grep '/.*\.log' -o)
	cur_f=$(basename $current_accessed_log_file)
	echo "Logging: $cur_f"

	if [ "${f}" != "${cur_f}" ]; then
		#echo "log: $f"
		f="${log_dir}/$f"
		dropbox_uploader/dropbox_uploader.sh -f ${conf_f} upload "$f" ${dropbox_dir}
		trash "$f"
	fi
done

popd
exit 0
