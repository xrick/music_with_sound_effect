#define MINIAUDIO_IMPLEMENTATION
#include "lib/miniaudio_0.11.21.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VHS_NOISE_LEVEL 0.01f        // Adjust to control noise level
#define VHS_WOW_FLUTTER_FREQ 0.5f    // Frequency of pitch modulation
#define WOW_FLUTTER_INTENSITY 0.005f // Intensity of wow/flutter

typedef struct {
    float pitchPhase;
} vhs_effect_data;

// Generate white noise
float generate_noise() {
    return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
}

// Apply VHS distortion (wow & flutter, noise)
void apply_vhs_effect(float* pSample, ma_uint32 sampleCount, vhs_effect_data* effectData, ma_uint32 sampleRate) {
    for (ma_uint32 i = 0; i < sampleCount; i++) {
        // Wow & flutter modulation (pitch)
        float wowFlutter = sinf(2 * M_PI * VHS_WOW_FLUTTER_FREQ * effectData->pitchPhase) * WOW_FLUTTER_INTENSITY;
        effectData->pitchPhase += 1.0f / (float)sampleRate;

        // Apply the wow & flutter effect to the sample
        pSample[i] *= (1.0f + wowFlutter);

        // Add white noise
        pSample[i] += generate_noise() * VHS_NOISE_LEVEL;

        // Ensure the sample stays within [-1.0, 1.0] range
        if (pSample[i] > 1.0f) pSample[i] = 1.0f;
        if (pSample[i] < -1.0f) pSample[i] = -1.0f;
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s <input wav file> <output wav file>\n", argv[0]);
        return -1;
    }

    const char* inputWavFile = argv[1];
    const char* outputWavFile = argv[2];

    srand((unsigned int)time(NULL));

    ma_result result;
    ma_decoder decoder;
    ma_encoder encoder;

    // Initialize the WAV decoder to read the input file
    result = ma_decoder_init_file(inputWavFile, NULL, &decoder);
    if (result != MA_SUCCESS) {
        printf("Failed to load WAV file: %s\n", inputWavFile);
        return -1;
    }

    // Initialize the WAV encoder to write the output file
    ma_encoder_config encoderConfig = ma_encoder_config_init(ma_encoding_format_wav, decoder.outputFormat, decoder.outputChannels, decoder.outputSampleRate);
    result = ma_encoder_init_file(outputWavFile, &encoderConfig, &encoder);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize output WAV file: %s\n", outputWavFile);
        ma_decoder_uninit(&decoder);
        return -1;
    }

    vhs_effect_data effectData;
    effectData.pitchPhase = 0;

    // Buffer to hold a chunk of audio samples
    ma_uint32 frameCount = 4096;
    float* pFrameBuffer = (float*)malloc(frameCount * decoder.outputChannels * sizeof(float));

    if (pFrameBuffer == NULL) {
        printf("Failed to allocate memory for audio processing.\n");
        ma_decoder_uninit(&decoder);
        ma_encoder_uninit(&encoder);
        return -1;
    }

    // Process and encode audio
    while (1) {
        ma_uint64 framesRead = ma_decoder_read_pcm_frames(&decoder, pFrameBuffer, frameCount, NULL);
        if (framesRead == 0) break;  // End of file

        // Apply the VHS distortion effect
        apply_vhs_effect(pFrameBuffer, (ma_uint32)framesRead * decoder.outputChannels, &effectData, decoder.outputSampleRate);

        // Write the processed audio frames to the output file
        ma_encoder_write_pcm_frames(&encoder, pFrameBuffer, framesRead, NULL);
    }

    // Clean up
    free(pFrameBuffer);
    ma_decoder_uninit(&decoder);
    ma_encoder_uninit(&encoder);

    printf("VHS distortion effect applied. Output saved to %s\n", outputWavFile);

    return 0;
}
