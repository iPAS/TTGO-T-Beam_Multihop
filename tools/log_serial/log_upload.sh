#!/bin/bash

here=$(dirname $(readlink -f "$0"))
source "${here}/env.sh"

# conf_f=/home/pi/.dropbox_uploader
# log_dir=log
# dropbox_dir=log/
pid_file="${here}/log_serial.pid"

for f in $(ls "${log_dir}"/*.log); do
	f=$(basename $f)
	echo "Log file: $f"

	if [ -f "${pid_file}" ]; then
		current_accessed_log_file=$(sudo lsof -p `cat "${pid_file}"` | grep '/.*\.log' -o)
		cur_f=$(basename "${current_accessed_log_file}")
	fi
	echo "Current accessed log (will be ignored): $cur_f"

	if [ "${f}" != "${cur_f}" ]; then
		#echo "log: $f"
		f="${log_dir}/$f"
		dropbox_uploader/dropbox_uploader.sh -f "${dropbox_conf}" upload "$f" "${dropbox_dir}"
		trash "$f"
	fi
done

exit 0

