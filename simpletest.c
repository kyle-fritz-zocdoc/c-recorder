/*
 * simpletest.c
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <mcp3004.h>

#define BASE 100
#define SPI_CHANNEL 0

int main(int argc, char **argv)
{
	wiringPiSetup();
	mcp3004Setup(BASE, SPI_CHANNEL);

    printf("|     0 |     1 |     2 |     3 |     4 |     5 |     6 |     7 |");
    printf("-----------------------------------------------------------------");

    int index;
    short channelValues[8];
    while (true) {
        for (index = 0; index < 8; index++) {
            channelValues[index] = analogRead(BASE + index);
        }
        for (index = 0; index < 8; index++) {
            printf("| %4i ", channelValues[index]);
        }
        printf("|\n");
    }

    return 0;
}