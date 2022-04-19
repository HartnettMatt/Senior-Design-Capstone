#include <iostream>
#include <string>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <errno.h>
#include <endian.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>

#include <clientManager.h>
#include <spiManager.h>
#include <gpsdoManager.h>


#define USER_REG_OFFSET         0x46000000
#define MAX_USER_REGS           0x31000 //Provide an upper bound for number of fpga registers needed (does not need to be strict)

//16384 sized
#define ADC_BRAM_OFFSET         0x00000
#define SEC_BRAM_OFFSET         0x10000
#define SAMP_BRAM_OFFSET        0x20000
#define RST_CTRL_OFFSET         0x30008
#define ACD_SNAP_CTRL_OFFSET    0x30014
#define SEC_SNAP_CTRL_OFFSET    0x30020
#define SAMP_SNAP_CTRL_OFFSET   0x3002c
#define LOCK_OFFSET             0x30028

#define SIZE_ADC_RAM_BUFFER     16384 //14 bits
#define SIZE_COUNT_RAM_BUFFERS  16384 //sample 27; seconds 32
#define SIZE_ADC_DATA           14
#define SIZE_SEC_DATA           32
#define SIZE_SAMP_DATA          27

uint EVENT_COUNTER = 0;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cerr << "USAGE ./ansible_client [MQTT BROKER IP]" << std::endl;
        exit(1);
    }

    srand(time(NULL));
    std::string brokerIP = argv[1];
    std::string BROKER_ADDRESS = "tcp://" + brokerIP + ":1883";
    std::string CLIENT_ID("clientNode" + std::to_string(rand()));
	clientManager myClient(BROKER_ADDRESS, CLIENT_ID);
	gpsdoManager myGPSDO;
	spiManager mySPI;

	std::cout << "Flashing N-PLL..." << std::endl;
    mySPI.initialize();
    sleep(1);
    std::cout << "N-PLL Ready!" << std::endl;

    std::cout << "Flashing fpga image..." << std::endl;
    system("cat fpga/top_16k_new.bit > /dev/xdevcfg"); //flash fpga image

	//get ptr offset
    int fd = open("/dev/mem", O_RDWR);
    void* ptr = mmap(NULL, MAX_USER_REGS, PROT_READ | PROT_WRITE, MAP_SHARED, fd, USER_REG_OFFSET);
    close(fd);

    //assign pointer offset
    volatile unsigned *rst_ctrl = (unsigned *)((uint8_t *)ptr + RST_CTRL_OFFSET);
    // volatile unsigned* seconds = (unsigned*)((uint8_t*)ptr + SECONDS_OFFSET);
    // volatile unsigned* samples = (unsigned*)((uint8_t*)ptr + SAMPLES_OFFSET);
    volatile unsigned *adc_bram = (unsigned *)((uint8_t *)ptr + ADC_BRAM_OFFSET);
    volatile unsigned *sec_bram = (unsigned *)((uint8_t *)ptr + SEC_BRAM_OFFSET);
    volatile unsigned *samp_bram = (unsigned *)((uint8_t *)ptr + SAMP_BRAM_OFFSET);
    volatile unsigned *adc_snap_ctrl = (unsigned *)((uint8_t *)ptr + ACD_SNAP_CTRL_OFFSET);
    volatile unsigned *sec_snap_ctrl = (unsigned *)((uint8_t *)ptr + SEC_SNAP_CTRL_OFFSET);
    volatile unsigned *samp_snap_ctrl = (unsigned *)((uint8_t *)ptr + SAMP_SNAP_CTRL_OFFSET);
    volatile unsigned *lock = (unsigned *)((uint8_t *)ptr + LOCK_OFFSET);

    std::cout << "FPGA Ready!" << std::endl;

	//arm snapshots
    std::cout << "Arming snapshot blocks..." << std::endl;
    for (int i = 0; i < 10; i++)
    {
        *adc_snap_ctrl = 1;
        *sec_snap_ctrl = 1;
        *samp_snap_ctrl = 1;
    }

    std::cout << "Waiting for GPS lock..." << std::endl;

    while (*lock==0)
    {
        lock = (unsigned*)((uint8_t*)ptr + LOCK_OFFSET);
    }

    *rst_ctrl = 0;
    usleep(1);
    *rst_ctrl = 1;
	std::string gpsFilePath = CLIENT_ID + "_GPS_data.txt";
	std::string gpsFileID = CLIENT_ID + "_GPS_data";
    myGPSDO.initialize(gpsFilePath);      //get time and location in node_data.txt

    std::cout << "\nReady!\n" << std::endl;

	try
    {
		myClient.connectToServer();
	}
	catch (const std::exception& e)
    {
		std::cerr << "MQTT CONNECTION ERROR: " << e.what() << std::endl;
	}

	myClient.registerFile(gpsFilePath, gpsFileID);
	myClient.initDataTransmit(gpsFileID);

    while (1)
    {
        if (adc_bram[0] > 0) //if data detected, save data
        {
            std::cout << "Event Detected\n" << std::endl;

            sleep(2);

            FILE *txt;
			std::string fileName = CLIENT_ID + "_node_data_event_" + std::to_string(EVENT_COUNTER) + ".csv";
			std::string fileID = CLIENT_ID + "_node_data_event_" + std::to_string(EVENT_COUNTER);
            txt = fopen(fileName.c_str(), "w+");

            //save data to .csv
            for (int i = 0; i < SIZE_COUNT_RAM_BUFFERS; i++)
            {
                fprintf(txt, "%d, %d, %d, %d\n", i, sec_bram[i], samp_bram[i], adc_bram[i]);
            }
            fclose(txt);
            std::cout << "Data collected." << std::endl;

            std::cout << "Clearing BRAM." << std::endl;
            //clear bram
            for (int i = 0; i < SIZE_COUNT_RAM_BUFFERS; i++)
            {
                adc_bram[i] = 0;
                sec_bram[i] = 0;
                samp_bram[i] = 0;
            }
            std::cout << "BRAM Cleared" << std::endl;

            //send node_data.txt and node_data.csv to server
			myClient.registerFile(fileName, fileID);
			myClient.initDataTransmit(fileID);
            EVENT_COUNTER++;
            std::cout << "Re-arming..." << std::endl;

            //re-arm snapshots
            *adc_snap_ctrl = 0;
            *sec_snap_ctrl = 0;
            *samp_snap_ctrl = 0;
            *adc_snap_ctrl = 1;
            *sec_snap_ctrl = 1;
            *samp_snap_ctrl = 1;

            std::cout << "\nReady!" << std::endl;
        }
    }

	std::cout << "CLOSING!" << std::endl;

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
