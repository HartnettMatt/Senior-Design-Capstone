# This code is for Keys On Kites boot up sequence
#!/bin/sh
rw
echo "Compiling code..."
gcc ./npll.c -o "./npll"
gcc ./gpsdo.c -o "./gps"
gcc ./gpio.c -o "./gpio"
gcc ./calibrate.c -o "./calibrate"
echo "Code compiled"
echo "Programming the NPLL"
./npll
# echo "Communicating with the GPS"
# ./gps
echo "Programming the FPGA"
./calibrate
echo "Preparing to trigger"
./gpio
echo "Reading the time tag:"
./timeoutput
