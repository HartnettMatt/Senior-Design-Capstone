# This code is for Keys On Kites Acceptance Test 2
#!/bin/sh
rw
echo "Compiling code..."
gcc ./npll.c -o "./npll"
gcc ./gpsdo.c -o "./gps"
gcc ./gpio.c -o "./gpio"
gcc ./timeOutput.c -o "timeoutput"
echo "Code compiled"

echo "Press any key to program the NPLL"
read -n 1 -s
echo "Programming the NPLL"
./npll

echo "Press any key to continue with the test (reading the GPS, programming the FPGA, and waiting for trigger)"
read -n 1 -s
echo "Communicating with the GPS"
./gps
echo "Programming the FPGA"
killall nginx
cat ./ddrdump_TRIG.bit > /dev/xdevcfg
nginx -p ../www/
echo "Running trigger script"
./gpio

echo "Reading the time tag:"
./timeoutput
