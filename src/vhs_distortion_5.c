#define MINIAUDIO_IMPLEMENTATION
#include "lib/miniaudio_0.11.21.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SAMPLE_RATE 44100
#define CHANNELS 1

#define PI 3.14159265358979323846
// #define FORMAT ma_format_s16
// VHS distortion parameters
#define FLUTTER_RATE 5.0f
#define FLUTTER_DEPTH 0.003f
#define NOISE_LEVEL 0.005f
#define WOW_RATE 0.5f
#define WOW_DEPTH 0.002f
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
    ma_result result;
    ma_decoder decoder;
    ma_encoder encoder;
    ma_uint64 totalFrames;
    ma_uint64 buffLength;

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
    totalFrames = ma_decoder_get_length_in_pcm_frames(&decoder, &buffLength);
    
    // Allocate memory for output samples
    outputSamples = (float*)malloc(totalFrames * decoder.outputChannels * sizeof(float));
    if (outputSamples == NULL) {
        printf("Failed to allocate memory for output samples.\n");
        ma_decoder_uninit(&decoder);
        return -3;
    }

    // Read PCM frames from the decoder
   
    // Initialize encoder for output WAV file
    ma_encoder_config config = ma_encoder_config_init(ma_encoding_format_wav, decoder.outputFormat, decoder.outputChannels, decoder.outputSampleRate);
    result = ma_encoder_init_file(argv[2], &config, &encoder);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize encoder: %d\n", result);
        free(outputSamples);
        ma_decoder_uninit(&decoder);
        return -4;
    }
     while(1){

        ma_uint64 framesRead = ma_decoder_read_pcm_frames(&decoder, outputSamples, totalFrames, NULL);
        if (framesRead == 0) break;  // End of file
        // apply_vhs_distortion(outputSamples, totalFrames, decoder.outputChannels, decoder.outputSampleRate);
        // Apply VHS effect to the output samples
        apply_vhs_effect(outputSamples, framesRead * decoder.outputChannels);
        // Write the processed audio frames to the output file
        // Write modified samples to the output WAV file
        // ma_encoder_write_pcm_frames(&encoder, pPCMFramesToWrite, framesToWrite, &framesWritten);
        // ma_encoder_write_pcm_frames(&encoder, pFrameBuffer, framesRead, NULL);
        ma_encoder_write_pcm_frames(&encoder, outputSamples, framesRead, NULL); 

    }
    
     

    // Clean up resources
    ma_encoder_uninit(&encoder);
    free(outputSamples);
    ma_decoder_uninit(&decoder);

    printf("VHS effect applied and saved to '%s'.\n", argv[2]);
    
    return 0;
}