#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_SAMPLE_VALUE 32767
#define PI 3.14159265358979323846

// VHS distortion parameters
#define FLUTTER_RATE 5.0f
#define FLUTTER_DEPTH 0.05f
#define NOISE_LEVEL 0.01f
#define WOW_RATE 0.5f
#define WOW_DEPTH 0.02f

// Function to apply VHS distortion effect
void apply_vhs_distortion(float* buffer, ma_uint64 frameCount, ma_uint32 channels, ma_uint32 sampleRate) {
    static float time = 0.0f;
    float timeStep = 1.0f / sampleRate;

    for (ma_uint64 i = 0; i < frameCount * channels; i += channels) {
        // Apply flutter effect
        float flutter = sinf(2.0f * PI * FLUTTER_RATE * time) * FLUTTER_DEPTH;

        // Apply wow effect
        float wow = sinf(2.0f * PI * WOW_RATE * time) * WOW_DEPTH;

        // Combine flutter and wow
        float distortion = 1.0f + flutter + wow;

        // Apply distortion and add noise
        for (ma_uint32 c = 0; c < channels; c++) {
            float noise = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * NOISE_LEVEL;
            buffer[i + c] = buffer[i + c] * distortion + noise;
            
            // Clamp the output to avoid overflow
            if (buffer[i + c] > 1.0f) buffer[i + c] = 1.0f;
            if (buffer[i + c] < -1.0f) buffer[i + c] = -1.0f;
        }

        time += timeStep;
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s <input_file.wav> <output_file.wav>\n", argv[0]);
        return -1;
    }

    const char* inputFile = argv[1];
    const char* outputFile = argv[2];

    ma_result result;
    ma_decoder decoder;
    ma_encoder encoder;
    ma_uint64 frameCount;
    ma_uint64 framesRead;
    float* pSampleData = NULL;

    result = ma_decoder_init_file(inputFile, NULL, &decoder);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize decoder.\n");
        return -1;
    }

    frameCount = ma_decoder_get_length_in_pcm_frames(&decoder);
    pSampleData = (float*)malloc(sizeof(float) * frameCount * decoder.outputChannels);

    framesRead = ma_decoder_read_pcm_frames(&decoder, pSampleData, frameCount);
    if (framesRead != frameCount) {
        printf("Failed to read all frames.\n");
        free(pSampleData);
        ma_decoder_uninit(&decoder);
        return -1;
    }

    // Apply VHS distortion effect
    apply_vhs_distortion(pSampleData, frameCount, decoder.outputChannels, decoder.outputSampleRate);

    // Initialize the encoder and write the processed audio data
    result = ma_encoder_init_file(outputFile, &decoder.outputFormat, &encoder);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize encoder.\n");
        free(pSampleData);
        ma_decoder_uninit(&decoder);
        return -1;
    }

    result = ma_encoder_write_pcm_frames(&encoder, pSampleData, frameCount);
    if (result != MA_SUCCESS) {
        printf("Failed to write frames to output file.\n");
    } else {
        printf("VHS distortion effect applied and saved to %s\n", outputFile);
    }

    // Clean up
    free(pSampleData);
    ma_decoder_uninit(&decoder);
    ma_encoder_uninit(&encoder);

    return 0;
}
