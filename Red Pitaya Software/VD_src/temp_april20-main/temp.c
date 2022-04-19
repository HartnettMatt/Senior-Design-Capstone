#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <errno.h>
#include <endian.h>
//#include <regmap.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>

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

int main()
{
    printf("\nExpo Demonstration\n\n");

    printf("\nFlashing N-PLL...");
    system("./spi");
    sleep(1);
    printf("\nN-PLL Ready!");
    printf("\nFlashing fpga image...");

    system("cat /root/temp_april20/top_16k_new.bit > /dev/xdevcfg"); //flash fpga image

    //get ptr offset
    int fd = open("/dev/mem", O_RDWR);
    void *ptr = mmap(NULL, MAX_USER_REGS, PROT_READ | PROT_WRITE, MAP_SHARED, fd, USER_REG_OFFSET);
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

    printf("\nFPGA Ready!");

    //arm snapshots
    printf("\nArming snapshot blocks and waiting for GPS lock...");
    for (int i = 0; i < 10; i++)
    {
        *adc_snap_ctrl = 1;
        *sec_snap_ctrl = 1;
        *samp_snap_ctrl = 1;
    }

    printf("\nWaiting for GPS lock...");

    while (*lock == 0)
    {
        lock = (unsigned *)((uint8_t *)ptr + LOCK_OFFSET);
    }

    *rst_ctrl = 0;
    usleep(1);
    *rst_ctrl = 1;
    system("./gpsdo"); //get time and location in node_data.txt
    printf("\nReady!\n");

    while (1)
    {

        if (adc_bram[0] > 0)
        { //if data detected, save data
            printf("\nEvent Detected\n");

            sleep(2);

            FILE *txt;
            txt = fopen("node_data.csv", "w+");

            //save data to .csv
            for (int i = 0; i < SIZE_COUNT_RAM_BUFFERS; i++)
            {
                fprintf(txt, "%d, %d, %d, %d\n", i, sec_bram[i], samp_bram[i], adc_bram[i]);
            }
            fclose(txt);
            printf("\nData collected.");

            printf("\nClearing BRAM.");
            //clear bram
            for (int i = 0; i < SIZE_COUNT_RAM_BUFFERS; i++)
            {
                adc_bram[i] = 0;
                sec_bram[i] = 0;
                samp_bram[i] = 0;
            }
            printf("\nBRAM Cleared");





            

            //MAX!! INSERT SENDING DATA HERE!!!







            printf("\nRe-arming...");

            //re-arm snapshots
            *adc_snap_ctrl = 0;
            *sec_snap_ctrl = 0;
            *samp_snap_ctrl = 0;
            *adc_snap_ctrl = 1;
            *sec_snap_ctrl = 1;
            *samp_snap_ctrl = 1;

            printf("\nReady!\n");
        }
    }
    return 0;
}
