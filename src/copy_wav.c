#define MA_DEBUG_OUTPUT
#define MINIAUDIO_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include "lib/miniaudio_0.11.21.h"

#define ERROR_ARGS 1
#define ERROR_DECODER_INIT 2
#define ERROR_DEVICE_INIT 3
#define ERROR_DEVICE_START 4
#define ERROR_READING_FRAMES 5
#define ERROR_READING_WAV_LENGTH 6
#define ERROR_OUTPUT_INIT 7

enum {ARG_FILENAME, ARG_INPUT_FILE, ARG_OUTPUT_FILE, ARG_AMP};

typedef struct {
	ma_decoder* decoder;
	double** buffer; // for testing
	ma_uint64* indexAddress;
	short* hasEnded;
} user_data;

//typedef void (* ma_device_data_proc)(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
//void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	user_data* pData = (user_data*)pDevice->pUserData;
	ma_decoder* pDecoder = (ma_decoder*)pData->decoder;
	double** buffer = (double**) pData->buffer;
	int start = *(pData->indexAddress);;
	ma_uint64 framesRead;
	ma_result result = ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, &framesRead);
	double* buffArr = *(buffer) + start;
	ma_copy_pcm_frames(buffArr , pOutput, frameCount, pDecoder->outputFormat, pDecoder->outputChannels);
	*(pData->indexAddress) += framesRead;
	if (result == MA_AT_END) {
		*(pData->hasEnded) = 0;
	}
	(void)pInput;
}

int main(int argc, char** argv) {
	ma_result result;
	ma_decoder decoder;
	double* outputBuffer;
	ma_uint64 trackingIndex = 0;
	ma_device device;
	ma_device_config deviceConfig;
	ma_uint64 buffLength;
	short hasEnded = 1;


	ma_encoder encoder;
	ma_encoder_config encoderConfig;


	char* inputFilePath = argv[ARG_INPUT_FILE];
	char* outputFilePath = argv[ARG_OUTPUT_FILE];
	float amp = atof(argv[ARG_AMP]);

	if (argc < 4) {
		printf("\n Usage: pan input output amp\n");
		return ERROR_ARGS;
	}


	result = ma_decoder_init_file(inputFilePath, NULL, &decoder);
	if (result != MA_SUCCESS) {
        printf("Failed to initialize decoder.\n");
		return ERROR_DECODER_INIT;
	}

	encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, decoder.outputFormat, decoder.outputChannels, decoder.outputSampleRate);

	if (ma_encoder_init_file(outputFilePath, &encoderConfig, &encoder) != MA_SUCCESS) {
        printf("Failed to initialize output file.\n");
        return ERROR_OUTPUT_INIT;
    }

	result = ma_decoder_get_length_in_pcm_frames(&decoder, &buffLength);
	if (result != MA_SUCCESS) {
        printf("Failed to get wav length.\n");
		return ERROR_READING_WAV_LENGTH;
	}

	outputBuffer = malloc(buffLength * sizeof(*outputBuffer));

	user_data userData = {&decoder, &outputBuffer, &trackingIndex, &hasEnded};
	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format = decoder.outputFormat;
	deviceConfig.playback.channels = decoder.outputChannels;
	deviceConfig.sampleRate = decoder.outputSampleRate;
	deviceConfig.dataCallback = dataCallback;
	deviceConfig.pUserData = &userData; // pass the decoder as user data

	if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
		printf("\n Initializing device failed\n");
		ma_decoder_uninit(&decoder);
		return ERROR_DEVICE_INIT;
	}

	if (ma_device_start(&device) != MA_SUCCESS) {
		printf("\n Failed to start device");
		ma_decoder_uninit(&decoder);
		ma_device_uninit(&device);
		return ERROR_DEVICE_START;
	}

	while (1) {
		if (hasEnded == 0) {
			break;
		};
	}

	if(ma_encoder_write_pcm_frames(&encoder, outputBuffer, buffLength, NULL) != MA_SUCCESS) {
		printf("Couldn't write");
		return -1;
	}

	printf("\n Number of elements in input: %llu, output buffer %llu", buffLength, trackingIndex);
	free(outputBuffer);
	ma_decoder_uninit(&decoder);
	ma_device_uninit(&device);
	ma_encoder_uninit(&encoder);

	return 0;
}