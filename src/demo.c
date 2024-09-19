#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define SAMPLE_RATE 44100
#define MAX_AMPLITUDE 32767
#define NOISE_LEVEL 0.05 // Adjust to control the amount of noise


// Function prototypes
void apply_vhs_effect(short *samples, size_t num_samples);

int main() {
    // Example audio data (replace with actual audio data)
    size_t num_samples = SAMPLE_RATE; // 1 second of audio
    short *samples = (short *)malloc(num_samples * sizeof(short));

    // Generate a simple sine wave as example audio
    for (size_t i = 0; i < num_samples; i++) {
        samples[i] = (short)(MAX_AMPLITUDE * sin(2 * M_PI * 440 * i / SAMPLE_RATE)); // 440 Hz sine wave
    }

    // Apply VHS effect
    apply_vhs_effect(samples, num_samples);

    // Save or process the distorted audio (code not shown)
    // For example, you might write it to a WAV file

    free(samples);
    return 0;
}

void apply_vhs_effect(short *samples, size_t num_samples) {
    // Apply noise
    for (size_t i = 0; i < num_samples; i++) {
        // Generate random noise
        short noise = (short)((rand() / (double)RAND_MAX - 0.5) * 2 * NOISE_LEVEL * MAX_AMPLITUDE);
        // Add noise to sample
        samples[i] += noise;

        // Simple low-pass filter (averaging over 5 samples)
        if (i >= 4) {
            samples[i] = (samples[i] + samples[i-1] + samples[i-2] + samples[i-3] + samples[i-4]) / 5;
        }
    }
}