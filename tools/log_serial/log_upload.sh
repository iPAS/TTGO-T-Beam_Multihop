#!/bin/bash

pushd .
cd /home/pi/log_serial
conf_f=/home/pi/.dropbox_uploader
#echo $cur_f

for f in `ls log/*.log`; do
	f=$(basename $f)
	echo "File: $f"
	
	current_accessed_log_file=$(sudo lsof -p `cat log_serial.pid` | grep '/.*\.log' -o)
	cur_f=$(basename $current_accessed_log_file)
	echo "Logging: $cur_f"

	if [ "${f}" != "${cur_f}" ]; then
		#echo "log: $f"
		f="log/$f"
		dropbox_uploader/dropbox_uploader.sh -f ${conf_f} upload "$f" log/ 
		trash "$f"
	fi
done

popd
exit 0
