1. At /home/pi, `> ln -sf /home/pi/TTGO-T-Beam_Multihop/tools/log_serial`.
  So, `/home/pi/log_serial` exist.

2. Go to /etc/monit/conf.d, `> ln -sf /home/pi/log_serial/log_serial.monit`.
  Then, `sudo service monit restart`, and, `sudo monit start log_serial`.

