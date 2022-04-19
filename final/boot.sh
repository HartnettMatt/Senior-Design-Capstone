# This code is for Keys On Kites boot up sequence
#!/bin/sh
rw
echo "Compiling code..."
gcc ./npll.c -o "./npll"
gcc ./gpsdo.c -o "./gpsdo"
gcc ./gpio.c -o "./gpio"
echo "Code compiled"
echo "Programming the NPLL"
./npll
echo "Communicating with the GPS"
./gps
echo "Programming the FPGA"
killall nginx
cat ./ddrdump_TRIG.bit > /dev/xdevcfg
nginx -p ../www/
"Preparing to trigger"
./gpio
