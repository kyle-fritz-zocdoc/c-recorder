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
	
	int sampleRateHz = 22 * 1000;
	int durationSec = 10;
	int sampleCount = durationSec * sampleRateHz;
	struct timespec sampleInterval;
	sampleInterval.tv_sec = 0;
	sampleInterval.tv_nsec = 1000000000L / sampleRateHz;
	
	int missThreshold = sampleCount / 5;
	int missCount = 0;
	
	int index;
	int values[sampleCount];
	struct timespec last, current, sampleReadDuration, delay;
	clock_gettime(CLOCK_REALTIME, &last);
	for (index = 0; index < sampleCount; index++)
	{
		values[index] = analogRead(BASE + ADC_CHANNEL);
		
		clock_gettime(CLOCK_REALTIME, &current);
		timespec_diff(&last, &current, &sampleReadDuration);
		timespec_diff(&sampleReadDuration, &sampleInterval, &delay);
		
		if (delay.tv_sec >= 0 && delay.tv_nsec >= 0) {
			nanosleep(&delay, (struct timespec *)NULL);
		}
		else {
			missCount++;
			
			if (missCount > missThreshold)
			{
				printf("Unable to read from device fast enough.\n");
				printf("last: %lld.%.9ld\n", (long long)last.tv_sec, last.tv_nsec);
				printf("current: %lld.%.9ld\n", (long long)current.tv_sec, current.tv_nsec);
				printf("sampleReadDuration: %lld.%.9ld\n", (long long)sampleReadDuration.tv_sec, sampleReadDuration.tv_nsec);
				printf("sampleInterval: %lld.%.9ld\n", (long long)sampleInterval.tv_sec, sampleInterval.tv_nsec);
				printf("delay: %lld.%.9ld\n", (long long)delay.tv_sec, delay.tv_nsec);
				printf("index: %i\n", index);
				return 1;
			}
		}
		
		last = current;
	}
	
	for (index = 0; index < sampleCount; index++)
	{
		printf("%i\n", values[index]);
	}
	
	return 0;
}

