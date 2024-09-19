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

int main(int argc, char** argv) {
    ma_result result;
    ma_decoder decoder;
    ma_encoder encoder;
    ma_uint64 totalFrames;
    float* outputSamples;
    
    if (argc < 3) {
        printf("Usage: %s <input.wav> <output.wav>\n", argv[0]);
        return -1;
    }

    srand((unsigned int)time(NULL)); // Seed for random noise

    // Initialize decoder for input WAV file
    result = ma_decoder_init_file(argv[1], NULL, &decoder);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize decoder: %d\n", result);
        return -2;
    }

    // Get total frame count
    totalFrames = ma_decoder_get_length_in_pcm_frames(&decoder);
    
    // Allocate memory for output samples
    outputSamples = (float*)malloc(totalFrames * decoder.outputChannels * sizeof(float));
    if (outputSamples == NULL) {
        printf("Failed to allocate memory for output samples.\n");
        ma_decoder_uninit(&decoder);
        return -3;
    }

    // Read PCM frames from the decoder
    ma_uint64 framesRead = ma_decoder_read_pcm_frames(&decoder, outputSamples, totalFrames, NULL);
    
    // Apply VHS effect to the output samples
    apply_vhs_effect(outputSamples, framesRead * decoder.outputChannels);

    // Initialize encoder for output WAV file
    result = ma_encoder_init_file(argv[2], ma_encoding_format_wav, 1, 44100, NULL, &encoder);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize encoder: %d\n", result);
        free(outputSamples);
        ma_decoder_uninit(&decoder);
        return -4;
    }

    // Write modified samples to the output WAV file
    ma_encoder_write_pcm_frames(&encoder, outputSamples, framesRead);

    // Clean up resources
    ma_encoder_uninit(&encoder);
    free(outputSamples);
    ma_decoder_uninit(&decoder);

    printf("VHS effect applied and saved to '%s'.\n", argv[2]);
    
    return 0;
}