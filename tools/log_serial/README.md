

1. At /home/pi, `> ln -sf /home/pi/TTGO-T-Beam_Multihop/tools/log_serial`.
  So, __/home/pi/log\_serial__ will exist, then:
	- `> cd ~/log_serial`
	- `> ./setup.sh`

2. Go to /etc/monit/conf.d, then:
	- `> ln -sf /home/pi/log_serial/log_serial.monit`
	- `> sudo service monit restart`
	- `> sudo monit start log_serial`

3. Setup uploading schedule:
	- `> crontab -e`
	- Enter `0 0 * * * /home/pi/log_serial/log_upload.sh`
