/*
 * recorder.c
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wiringPi.h>
#include <mcp3004.h>
#include <string.h>

#define BASE 100
#define SPI_CHANNEL 0
#define ADC_CHANNEL 7

// wav file builder - begin

struct wavfile_header {
	char	riff_tag[4];
	int	    riff_length;
	char	wave_tag[4];
	char	fmt_tag[4];
	int	    fmt_length;
	short	audio_format;
	short	num_channels;
	int	    sample_rate;
	int	    byte_rate;
	short	block_align;
	short	bits_per_sample;
	char	data_tag[4];
	int	    data_length;
};

FILE * wavfile_open( const char *filename, int samplerate)
{
	struct wavfile_header header;

	int samples_per_second = samplerate;
	short bits_per_sample = 16;
	short block_align = (bits_per_sample + 7) / 8; // Add seven so that the divide always rounds up.

	strncpy(header.riff_tag, "RIFF", 4);
	strncpy(header.wave_tag, "WAVE", 4);
	strncpy(header.fmt_tag, "fmt ", 4);
	strncpy(header.data_tag, "data", 4);

	header.riff_length = 0;
	header.fmt_length = 16;
	header.audio_format = 1;
	header.num_channels = 1;
	header.sample_rate = samples_per_second;
	header.byte_rate = (samples_per_second * bits_per_sample + 7) / 8; // Add seven so that the divide always rounds up.
	header.block_align = block_align;
	header.bits_per_sample = bits_per_sample;
	header.data_length = 0;

	FILE * file = fopen(filename, "w+");
	if(!file) return 0;

	fwrite(&header,sizeof(header), 1, file);

	fflush(file);

	return file;

}

void wavfile_write( FILE *file, short data[], int length )
{
	fwrite(data,sizeof(short),length,file);
}

void wavfile_close( FILE *file )
{
	int file_length = ftell(file);

	int data_length = file_length - sizeof(struct wavfile_header);
	fseek(file,sizeof(struct wavfile_header) - sizeof(int),SEEK_SET);
	fwrite(&data_length,sizeof(data_length),1,file);

	int riff_length = file_length - 8;
	fseek(file,4,SEEK_SET);
	fwrite(&riff_length,sizeof(riff_length),1,file);

	fclose(file);
}
// wav file builder -end

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

void input2wavbytes(int* input, short* wav, int sz)
{
	if(!wav)
	{
		wav = malloc(sz*sizeof(short));
	}
	// need to add the logic to convert 
	// temporarily copying input to wav
	
	memcpy(wav, input, sz*sizeof(int));
	
}

int main(int argc, char **argv)
{
	wiringPiSetup();
	mcp3004Setup(BASE, SPI_CHANNEL);
	
	int sampleCount = 22 * 1000 * 10;
	
	int index;
	short values[sampleCount];
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
		// Sloppy upscale from 10-bit to 16-bit.
		values[index] = (short)(values[index] * 1.6);
	}
	
	FILE* file = wavfile_open("sound.wav", (int)sampleRateHz);
	if(!file)
	{
		printf("Could not open file sound.wav");
		return 0;
	}

	wavfile_write(file, values, sampleCount);
	wavfile_close(file);
	
	return 0;
}

