#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#define MAXCHAR 1000
#define TIMELENGTH 6



int main(int argc, char *argv[])
{
    FILE *fp;
    FILE *fpAlt;
    char row[MAXCHAR];

    fp = fopen("gpsdata.csv","r");

    char check[5] = "GPZDA";
    char time[6];
    int hours;
    int min;
    int seconds;
    while (feof(fp) != true)
    {
        char subRow[5] = "";
        fgets(row, MAXCHAR, fp);
        memcpy(subRow,&row[1], sizeof(check));

        if(strncmp(subRow, check, 5) == 0){
            memcpy(time,&row[7], 2);
            hours = atoi(time);
            memcpy(time,&row[9], 2);
            min = atoi(time);
            memcpy(time,&row[11], 2);
            seconds = atoi(time);

        }

    }
    fclose(fp);
    fpAlt = fopen("timetag.csv","r");
    char buffer[1024];

    int row_ = 0;
    int column = 0;
    int ns;
    int sec;

    while (fgets(buffer,
                    1024, fpAlt)) {

            column = 0;
            row_++;

            char* value = strtok(buffer, ", ");

            while (value) {
                // Column 1
                if (column == 0) {
                    ns = atoi(value)*8;
                }

                // Column 2
                if (column == 1) {
                    sec = atoi(value);
                }


                value = strtok(NULL, ", ");
                column++;
            }


        }
    fclose(fpAlt);
    sec = sec + seconds;
    if(ns / 1000000 >= 1){
        sec += (int)(ns/1000000);
        ns = ns % 1000000;
    }

    if(sec/60 >= 1){
        min += (int)(sec/60);
        sec = sec % 60;
    }
    if(min/60 >= 1){
        hours += (int)(min/60);
        min = min % 60;
    }
    if(hours >= 24){
        hours = hours - 24;
    }
    printf("Hours: %d\n", hours);
    printf("Minutes: %d\n", min);
    printf("Seconds: %d.", sec);
    printf("%d\n",ns);
    printf("\n");

    printf("UTC TIME- %d:%d:%d.%d", hours, min, sec, ns);
    printf("\n");


    return 0;

}
