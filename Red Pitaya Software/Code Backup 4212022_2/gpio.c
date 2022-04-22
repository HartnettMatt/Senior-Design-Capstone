#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define PIN 976
#define POUT 968

#define VALUE_MAX 30
#define BUFFER_MAX 3

#define MAX_PATH 64

#define TRIGG 0

static int pin_export(int pin)
{
	char shell[MAX_PATH];
	sprintf(shell,"echo %d > /sys/class/gpio/export", pin);
	system(shell);
	return 0;
}

static int pin_unexport(int pin)
{
  char shell[MAX_PATH];
  sprintf(shell,"echo %d > /sys/class/gpio/unexport", pin);
  system(shell);
	return 0;
}

static int pin_direction(int pin, int dir){
	char shell[MAX_PATH];
	snprintf(shell, MAX_PATH, "echo %s > /sys/class/gpio/gpio%d/direction",((dir==IN)?"in":"out"),pin);
	system(shell);
	return 0;
}

static int pin_read(int pin){
	char path[VALUE_MAX];
	char value_str[3];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);

	// get pin file descriptor for reading its state
	fd = open(path, O_RDONLY);
	if (-1 == fd) {
		fprintf(stderr, "Unable to open gpio sysfs pin value file %s for reading\n",path);
		return -1;
	}

	// read value
	if (-1 == read(fd, value_str, 3)) {
		fprintf(stderr, "Unable to read value\n");
		return -1;
	}

	// close file
	close(fd);

	// return integar value
	return atoi(value_str);
}

static int pin_write(int pin, int value) {
	char path[VALUE_MAX];
	int fd;
	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	// get pin value file descrptor
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Unable to to open sysfs pins value file %s for writing\n",path);
		return -1;
	}
	if(value==LOW){
		//write low
		if (1 != write(fd, "0", 1)) {
			fprintf(stderr, "Unable to write value\n");
			return -1;
		}
	} else if(value==HIGH) {
		//write high
		if (1 != write(fd, "1", 1)) {
      fprintf(stderr, "Unable to write value\n");
      return -1;
		}
	} else {
		fprintf(stderr, "Nonvalid pin value requested\n");
	}
	//close file
	close(fd);
	return 0;
}

int main(int argc, char *argv[]) {
	printf("Waiting for trigger...\n");
	unsigned short second = 0b0;
	unsigned long cycle = 0b0;
	char command[65];
	char awsCom1[200];
	char awsCom2[200];
	char gpsCom1[20];
	char gpsCom2[6];
	strcpy(command, "./rp_remote_acquire -m file -k 24413 -c 0 -d 1 -f /vol/out1.dat");
	strcpy(awsCom1, "scp -i /KeyOnKites.pem /opt/redpitaya/tmp/timetag.csv ubuntu@ec2-3-23-168-123.us-east-2.compute.amazonaws.com:~");
	strcpy(awsCom2, "scp -i /KeyOnKites.pem /opt/redpitaya/tmp/gpsdata.csv ubuntu@ec2-3-23-168-123.us-east-2.compute.amazonaws.com:~");
	strcpy(gpsCom1, "gcc gpsdo.c -o gps");
	strcpy(gpsCom2, "./gps");
	while(1){
		pin_export(960);
		pin_direction(960, IN);
		if(pin_read(960)==TRIGG){
			pin_unexport(960);
			printf("Trigger High\n");
			system(command);

			// Read the cycle counter
			for (int i = 999; i >= 973; i--){
				pin_export(i);
				pin_direction(i, IN);
				cycle =  cycle | pin_read(i);
				if(i != 973){
					cycle = cycle<<1;
				}
				pin_unexport(i);
			}

			// Read the second counter
			for(int i = 972; i >= 962; i--){
				pin_export(i);
				pin_direction(i, IN);
				second = second | pin_read(i);
				if(i != 962){
					second = second<<1;
				}
				pin_unexport(i);
			}
			//system(gpsCom1);
			//system(gpsCom2);
			// Open the time tag data file and populate it
			FILE *fpt;
			fpt = fopen("timetag.csv", "w+");
			fprintf(fpt, "%u, %lu", second, cycle);
			fclose(fpt);

			printf("timetag.csv written to!\n");
			printf("Uploading GPS and time tag data");
			system(awsCom1);
			system(awsCom2);
			printf("Check server.\n");
			second = 0b0;
			cycle = 0b0;
			break;
		} else {
			pin_unexport(960);
		}
	}
	return 0;
}
