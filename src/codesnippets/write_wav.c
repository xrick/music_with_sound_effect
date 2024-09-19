#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <math.h>
#include <stdio.h>

#define SAMPLE_RATE 44100       // 44.1kHz sample rate
#define FREQUENCY 440.0         // Frequency of A4 note (440 Hz)
#define DURATION_SECONDS 5      // 5 seconds duration
#define AMPLITUDE 0.5           // Amplitude of the sine wave (50% volume)

int main() {
    // Define encoder configuration (PCM 16-bit, 44.1kHz, mono)
    ma_encoder_config config = ma_encoder_config_init(ma_resource_format_wav, ma_format_s16, 1, SAMPLE_RATE);

    // Initialize the encoder (output file, format configuration)
    ma_encoder encoder;
    if (ma_encoder_init_file("output.wav", &config, &encoder) != MA_SUCCESS) {
        printf("Failed to initialize WAV encoder.\n");
        return -1;
    }

    // Allocate a buffer to hold audio samples
    int totalSamples = SAMPLE_RATE * DURATION_SECONDS;
    short* pBuffer = (short*)malloc(totalSamples * sizeof(short));
    if (pBuffer == NULL) {
        printf("Failed to allocate memory for audio buffer.\n");
        ma_encoder_uninit(&encoder);
        return -1;
    }

    // Generate a sine wave (A4 tone, 440 Hz)
    for (int i = 0; i < totalSamples; i++) {
        float t = (float)i / (float)SAMPLE_RATE;               // Time in seconds
        float sineValue = sinf(2.0f * M_PI * FREQUENCY * t);   // Sine wave formula
        pBuffer[i] = (short)(AMPLITUDE * 32767.0f * sineValue); // Scale to 16-bit range
    }

    // Write the buffer to the WAV file
    ma_encoder_write_pcm_frames(&encoder, pBuffer, totalSamples);

    // Clean up
    free(pBuffer);
    ma_encoder_uninit(&encoder);

    printf("WAV file created: output.wav\n");
    return 0;
}
