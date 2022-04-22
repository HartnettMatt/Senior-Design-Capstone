/* Reiko Matsuda-Dunn
* Based on uart.c from Luka Golinar <luka.golinar@redpitaya.com>
* This talks to the GPSDO via UART. No writing to the UART is required in this version.
* Make with the makefile from the RP git repo:
* 	$cd RedPitaya/Examples/C
*	$make gpsdo
*	$LD_LIBRARY_PATH=/opt/redpitaya/lib ./gpsdo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>                 //Used for UART
#include <fcntl.h>                  //Used for UART
#include <termios.h>                //Used for UART
#include <errno.h>

#define MAX_SENTENCE 300
#define BAUD_RATE_INIT B38400
#define BAUD_RATE_COMM B4800

/* Inline function definition */
static int uart_init();
static int release();
static int uart_read();

/* File descriptor definition */

int uart_fd = -1;

static int uart_init(){
    //more info on tty drivers here: http://www.linusakesson.net/programming/tty/index.php
    //read&write | port never controls processe | non-blocking I/O
    uart_fd = open("/dev/ttyPS1", O_RDWR | O_NOCTTY | O_NDELAY);

    if(uart_fd == -1){
        fprintf(stderr, "Failed to open uart.\n");
        return -1;
    }

    struct termios settings;
    tcgetattr(uart_fd, &settings);

   /*  CONFIGURE THE UART
   *  The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
    *       Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
    *       CSIZE:- CS5, CS6, CS7, CS8
    *       CLOCAL - Ignore modem status lines
    *       CREAD - Enable receiver
    *       IGNPAR = Ignore characters with parity errors
    *       ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
    *       PARENB - Parity enable
    *       PARODD - Odd parity (else even) */


    /* Set baud rate - default set to 9600Hz */
    speed_t baud_rate = BAUD_RATE_INIT;

    /* Baud rate fuctions
    * cfsetospeed - Set output speed
    * cfsetispeed - Set input speed
    * cfsetspeed  - Set both output and input speed */

    cfsetspeed(&settings, baud_rate);

    /*  c_cflag : control flags
    *   c_oflag : output flags
    *   c_iflag : input flags
    *   c_lflag : local flags
    * */

    settings.c_cflag &= ~PARENB; /* no parity */
    settings.c_cflag &= ~CSTOPB; /* 1 stop bit */
    settings.c_cflag &= ~CSIZE;
    settings.c_cflag |= CS8 | CLOCAL; /* 8 bits */
    settings.c_lflag = ICANON; /* canonical mode */ //input is made available line by line
    settings.c_oflag &= ~OPOST; /* raw output */

    /* Setting attributes */
    tcflush(uart_fd, TCIFLUSH);
    tcsetattr(uart_fd, TCSANOW, &settings);

    return 0;
}

static int uart_read(int read_count){
  /* Read some sample data from RX UART */
  /* Don't block serial read */
  fcntl(uart_fd, F_SETFL, FNDELAY);
  while(1){
    if(uart_fd == -1){
      fprintf(stderr, "Failed to read from UART.\n");
      return -1;
    }

    unsigned char rx_buffer[MAX_SENTENCE];
    int count = 0;
    int rx_length = read(uart_fd, (void*)rx_buffer, MAX_SENTENCE);

    if (rx_length < 0){
      /* No data yet avaliable, check again */
      if(errno == EAGAIN){
          //fprintf(stderr, "AGAIN!\n");
          continue;
      /* Error differs */
      } else {
         fprintf(stderr, "Error!\n");
         return -1;
      }
    } else if (rx_length == 0){
      fprintf(stderr, "No data waiting\n");
      /* Print data and exit while loop */
  	} else {
      if ((int)rx_buffer[count] != 36){ // not start sequence!
         continue;
      } else {
        while((int)rx_buffer[count]!= 10){
          count ++;
        }
        rx_buffer[count] = '\0';
        //printf("%i bytes read : %s\n", count, rx_buffer);
        // open file
        FILE *txt;
        if(read_count== 0){
          txt = fopen("gpsdata.csv", "w+");
        } else {
          txt = fopen("gpsdata.csv", "a");
        }
        fprintf(txt,"%s\n",rx_buffer);
        printf("%s\n",rx_buffer);
        fclose(txt);
        break;
      }
    }
  }
  return 0;
}

static int uart_write(char *data){

        /* Write some sample data into UART */
        /* ----- TX BYTES ----- */
        int msg_len = strlen(data);

        int count = 0;
        char tx_buffer[msg_len+1];

        strncpy(tx_buffer, data, msg_len);
        tx_buffer[msg_len++] = 0x0a; //New line numerical value

        if(uart_fd != -1){
                count = write(uart_fd, &tx_buffer, (msg_len));
        }
        if(count < 0){
                fprintf(stderr, "UART TX error.\n");
                return -1;
        }

        return 0;
}

static int release(){
    tcflush(uart_fd, TCIFLUSH);
    close(uart_fd);
    return 0;
}


int main(int argc, char *argv[]){
//	char *data = "$PERDAPI, GCLK,1,25000000,50,00*41";
//	char *data = "$PERDAPI,SURVEY,1,10,1440*74";
//	char *data = "$PERDAPI,SURVEY,0,0,0,37.7870,-122.4510,31*48";
	char data[100] = "$PERDCRY,TPS3,3,003,001,002205,000010,0,0,00,0x00000001,0x0000000*0D";
	printf("Initializing UART ...\n");

    struct termios attributes;
    if(tcgetattr(STDIN_FILENO, &attributes) < 0) {
        perror("stdin");
        return EXIT_FAILURE;
    }

    // speed_t current_baud = cfgetispeed(&attributes);
    // printf("current baud rate : %d \n", current_baud);

    /* uart init */
  if(uart_init() < 0){
      printf("Uart init error.\n");
      return -1;
  }
	if(uart_write(data) < 0) { // write function
		printf("error stupid");
		return -1;
	}


    /* read */
    for(int i = 0; i<20 ; i++){
        if(uart_read(i) < 0){
            printf("Uart read error\n");
            return -1;
        }
    }

	// read

    /* CLOSING UART */
    release();
    return 0;
}
