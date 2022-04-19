#ifndef SPI_MANAGER
#define SPI_MANAGER

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>	
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>

class spiManager{
    public:
        spiManager();
        int initialize();
    private:
        int m_spi_fd = -1;
        int init_spi();
        int release_spi();
        int write_spi(void *write_data, int size);
};

#endif