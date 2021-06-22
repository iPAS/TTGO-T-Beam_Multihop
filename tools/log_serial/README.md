1. At /home/pi, `> ln -sf /home/pi/TTGO-T-Beam_Multihop/tools/log_serial`.
  So, __/home/pi/log\_serial__ exist.

2. Go to /etc/monit/conf.d, then:
	- `> ln -sf /home/pi/log_serial/log_serial.monit`
	- `> sudo service monit restart`
	- `> sudo monit start log_serial`

3. Go to /etc/cron.daily, then:
	- `> ln -sf /home/pi/log_serial/log_upload.sh log_upload`

