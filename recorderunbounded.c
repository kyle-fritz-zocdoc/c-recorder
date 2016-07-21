/*
 * recorder.c
 * 
 */


#include <stdio.h>
#include <time.h>
#include <wiringPi.h>
#include <mcp3004.h>

#define BASE 100
#define SPI_CHANNEL 0
#define ADC_CHANNEL 7

void timespec_diff(struct timespec *start, struct timespec *stop,
                   struct timespec *result)
{
	if ((stop->tv_nsec - start->tv_nsec) < 0)
	{
		result->tv_sec = stop->tv_sec - start->tv_sec - 1;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
	}
	else
	{
		result->tv_sec = stop->tv_sec - start->tv_sec;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec;
	}
	return;
}

int main(int argc, char **argv)
{
	wiringPiSetup();
	mcp3004Setup(BASE, SPI_CHANNEL);
	
	int sampleCount = 22 * 1000 * 10;
	
	int index;
	int values[sampleCount];
	struct timespec start, end, duration;
	clock_gettime(CLOCK_REALTIME, &start);
	for (index = 0; index < sampleCount; index++)
	{
		values[index] = analogRead(BASE + ADC_CHANNEL);
	}
	clock_gettime(CLOCK_REALTIME, &end);
	timespec_diff(&start, &end, &duration);
	double sampleRateHz = (1.0 * sampleCount) / (duration.tv_sec + (duration.tv_nsec / 1000000000.0));
	printf("duration: %lld.%.9ld\n", (long long)duration.tv_sec, duration.tv_nsec);
	printf("sample rate: %f\n", sampleRateHz);
	
	for (index = 0; index < sampleCount; index++)
	{
		printf("%i\n", values[index]);
	}
	
	return 0;
}

