/* @brief This is a simple application for testing SPI communication on a RedPitaya
 * @Author Luka Golinar <luka.golinar@redpitaya.com>
 * 
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <spiManager.h>


spiManager::spiManager()
{}

int spiManager::initialize()
{
    /* Sample data */
	uint32_t hex1 = 0xe6738012;
	void* data1 = (void*)&hex1;

    uint32_t hex2 = 0xe6738012;
    void* data2 = (void*)&hex2;

    uint32_t hex3 = 0x58020006;
    void* data3 = (void*)&hex3;

    uint32_t hex4 = 0x2018000a;
    void* data4 = (void*)&hex4;

    uint32_t hex5 = 0x309f000a;
    void* data5 = (void*)&hex5;

    uint32_t hex6 = 0x0040000a;
    void* data6 = (void*)&hex6;

	/* Init the spi resources */
	if(init_spi() < 0)
    {
		printf("Initialization of SPI failed. Error: %s\n", strerror(errno));
		return -1;
	}
/////////////////////////////////////////////////////////////////////////////////////////////////
	/* Write some sample data */
	if(write_spi(data1, 4) < 0)
    {
		printf("Write to SPI failed. Error: %s\n", strerror(errno));
		return -1;
	}


    if(write_spi(data2, 4) < 0)
    {
            printf("Write to SPI failed. Error: %s\n", strerror(errno));
            return -1;
    }



    if(write_spi(data3, 4) < 0)
    {
            printf("Write to SPI failed. Error: %s\n", strerror(errno));
            return -1;
    }



    if(write_spi(data4, 4) < 0)
    {
            printf("Write to SPI failed. Error: %s\n", strerror(errno));
            return -1;
    }


    if(write_spi(data5, 4) < 0)
    {
            printf("Write to SPI failed. Error: %s\n", strerror(errno));
            return -1;
    }



    if(write_spi(data6, 4) < 0)
    {
            printf("Write to SPI failed. Error: %s\n", strerror(errno));
            return -1;
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* Release resources */
	if(release_spi() < 0)
    {
		printf("Relase of SPI resources failed, Error: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int spiManager::init_spi(){

	/* MODES: mode |= SPI_LOOP; 
	 *        mode |= SPI_CPHA; 
	 *        mode |= SPI_CPOL; 
	 *		  mode |= SPI_LSB_FIRST; 
	 *        mode |= SPI_CS_HIGH; 
	 *        mode |= SPI_3WIRE; 
	 *        mode |= SPI_NO_CS; 
	 *        mode |= SPI_READY;
	 *
	 * multiple possibilities possible using | */
	uint8_t mode=0;

	/* Opening file stream */
	m_spi_fd = open("/dev/spidev1.0", O_RDWR | O_NOCTTY);
	if(m_spi_fd < 0)
    {
		printf("Error opening spidev0.1. Error: %s\n", strerror(errno));
		return -1;
	}

	/* Setting mode (CPHA, CPOL) */
	if(ioctl(m_spi_fd, SPI_IOC_WR_MODE, &mode) < 0)
    {
		printf("Error setting SPI MODE. Error: %s\n", strerror(errno));
		return -1;
	}

	/* Setting SPI bus speed */
	int spi_speed = 1000000;

	if(ioctl(m_spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0)
    {
		printf("Error setting SPI MAX_SPEED_HZ. Error: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

int spiManager::release_spi()
{
	/* Release the spi resources */
	close(m_spi_fd);

	return 0;
}

/* Write data to the SPI bus */
int spiManager::write_spi(void *write_buffer, int size)
{
	int write_spi = write(m_spi_fd, write_buffer, size);

	if(write_spi < 0)
    {
		printf("Failed to write to SPI. Error: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}