#define MINIAUDIO_IMPLEMENTATION
#include "lib/miniaudio_0.11.21.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void apply_vhs_effect(float* samples, ma_uint32 frameCount) {
    for (ma_uint32 i = 0; i < frameCount; i++) {
        // Introduce noise
        float noise = ((float)rand() / RAND_MAX) * 0.05f; // Adjust noise level
        samples[i] += noise;

        // Simulate pitch modulation (simple example)
        if (i % 44100 == 0) { // Every second
            samples[i] *= 0.98f; // Slightly lower pitch
        }

        // Apply low-pass filter (simple averaging)
        if (i > 0) {
            samples[i] = (samples[i] + samples[i - 1]) / 2.0f; // Basic low-pass filter
        }
    }
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    float* outputSamples = (float*)pOutput;
    ma_decoder_read_pcm_frames(pDecoder, outputSamples, frameCount, NULL);
    
    // Apply VHS effect to the output samples
    apply_vhs_effect(outputSamples, frameCount);
}

int main(int argc, char** argv) {
    ma_result result;
    ma_decoder decoder;
    ma_device_config deviceConfig;
    ma_device device;

    if (argc < 2) {
        printf("Usage: %s <input.wav>\n", argv[0]);
        return -1;
    }

    srand((unsigned int)time(NULL)); // Seed for random noise

    result = ma_decoder_init_file(argv[1], NULL, &decoder);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize decoder: %d\n", result);
        return -2;
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate = decoder.outputSampleRate;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = &decoder;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
        return -3;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
        return -4;
    }

    printf("Playing '%s'. Press Enter to quit...\n", argv[1]);
    getchar();

    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);

    return 0;
}