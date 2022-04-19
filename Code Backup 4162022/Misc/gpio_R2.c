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

#define TRIGG 1

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

static int pin_write(int pin, int value)
{
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
	}
        else if(value==HIGH){
		//write high
		if (1 != write(fd, "1", 1)) {
                	fprintf(stderr, "Unable to write value\n");
                	return -1;
		}
	}else fprintf(stderr, "Nonvalid pin value requested\n");

	//close file
	close(fd);
	return 0;
}

int main(int argc, char *argv[])
{
	printf("Entered Main\n");
	unsigned short bottom = 0b0;
	unsigned long top = 0b0;
	char command[65];
	char awsCom[200];
	char gpsCom1[20];
	char gpsCom2[6];
	strcpy(command, "./rp_remote_acquire -m file -k 24413 -c 0 -d 1 -f /vol/out1.dat");
	strcpy(awsCom, "scp -i /KeyOnKites.pem /opt/redpitaya/tmp/timeDump.csv ubuntu@ec2-3-23-168-123.us-east-2.compute.amazonaws.com:~");
	strcpy(gpsCom1, "gcc gpsdo.c -o gps");
	strcpy(gpsCom2, "./gps");
	pin_export(960);
	pin_direction(960,IN);
	while(1){
        //pin_export(960);
        //pin_direction(960, IN);
       // if(pin_read(960)==TRIGG){
          
			pin_unexport(960);
			printf("Trigger High\n");
			//system(command);
			for (int i = 999; i >= 987; i--){
				pin_export(i);
				pin_direction(i, IN);
				//pin_read(i);
				printf("Pin %i : %i\n",i,pin_read(i));
				bottom = bottom | pin_read(i);
				if(i != 987){
					bottom = bottom<<1;
				}
				pin_unexport(i);
			}
			for(int i = 986; i >= 960; i--){
				pin_export(i);
				pin_direction(i, IN);
				printf("Pin %i, : %i\n", i, pin_read(i));
				//pin_read(i);
				top = top | pin_read(i);
				if(i != 960){
					top = top<<1;
				}
				pin_unexport(i);
			}
			//system(gpsCom1);
			//system(gpsCom2);

			FILE *fpt;
			fpt = fopen("timeDump.csv", "w+");
			fprintf(fpt, "%u, %lu", bottom, top);
			fclose(fpt);
			printf("timeDump.csv written to!\n");

			system(awsCom);
			printf("Check server.\n");
			bottom = 0b0;
			top =0b0;
			break;
       // }
	//	else{
			//pin_unexport(960);
	//	}
		

    }
	
	return 0;
}
