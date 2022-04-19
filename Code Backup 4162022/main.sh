#!/bin/sh
rw
echo "Compiling code..."
gcc ./spi_W.c -o "./spi"
gcc ./gpsdo_backup.c -o "./gps"
gcc ./gpio_R.c -o "./gpio"
echo "Code compiled"
echo "Programming the NPLL"
./spi
echo "Communicating with the GPS"
./gps
echo "Programming the FPGA"
killall nginx
cat ./red_pitaya_top.bit > /dev/xdevcfg
nginx -p ../www/
"Preparing to trigger"
./gpio
