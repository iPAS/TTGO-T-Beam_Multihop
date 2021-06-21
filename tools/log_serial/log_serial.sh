#!/bin/bash
 
source /home/pi/.virtualenvs/deep/bin/activate

#here="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
here=/home/pi/log_serial

serial_options='-d /dev/ttyUSB0 -b 115200'
log_period_min=60

stamp_format='%Y-%m-%d %H:%M:%S.%f'

filename_format='lora_relay_%Y-%m-%d_%H:%M:%S.log'
#filename_format='lora_relay.log'

rotate="--endtime=$((log_period_min * 60)) --again"

log_dir=log
log_file="--append --output=${here}/${log_dir}/${filename_format}"

script_name=$(basename $0 .sh)

pid_file="${here}/${script_name}.pid"




case $1 in
start)
	grabserial  ${serial_options}  --timeformat="${stamp_format}" --systime  ${log_file}  ${rotate}  --quiet  &
	#echo $$ > ${pid_file}
	echo $! > ${pid_file}
	;;

run)
	grabserial  ${serial_options}  --timeformat="${stamp_format}" --systime  ${log_file}  ${rotate}
	;;

stop)
	kill `cat ${pid_file}` && rm -f ${pid_file}
	;;

status)
	[[ ! -f "${pid_file}" ]] && exit 1
	;;

*)
	echo "usage: $0 {start|run|stop}" >&2
	exit 1
	;;
esac
exit 0

