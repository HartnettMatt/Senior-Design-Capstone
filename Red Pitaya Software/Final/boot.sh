# This code is for Keys On Kites Acceptance Test 2
#!/bin/sh
rw
echo "Compiling code..."
gcc ./npll.c -o "./npll"
gcc ./gpsdo.c -o "./gps"
gcc ./gpio.c -o "./gpio"
gcc ./timeOutput.c -o "timeoutput"
gcc ./calibrate.c -o "calibrate"
echo "Code compiled"

#echo "Press any key to program the NPLL"
#read -n 1 -s
echo "Programming the NPLL"
./npll

#echo "Press any key to continue with the test (reading the GPS, programming the FPGA, and waiting for trigger)"
#read -n 1 -s
echo "Running the calibrate"
./calibrate


echo "calibration complete"
./gpio
echo "Reading the time tag:"
./timeoutput
