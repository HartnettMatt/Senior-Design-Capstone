#ifndef GPSDO_MANAGER
#define GPSDO_MANAGER
#include <string>
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

class gpsdoManager{
    public:
        gpsdoManager();
        int initialize(std::string filePath);
    private:
        int m_uart_fd = -1;
        int uart_init();
        int release();
        int uart_read(int read_count, std::string filePath);
};


#endif