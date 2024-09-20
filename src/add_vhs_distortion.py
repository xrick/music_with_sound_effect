import sys
import miniaudio
import numpy as np

def add_vhs_distortion(input_file=None, output_file=None, noise_lel=0.00005):
    # Decode the input WAV file
    audio_data = miniaudio.decode_file(input_file)

    # Apply VHS distortion effect
    noise_level = noise_lel  # Adjust noise level for distortion
    num_samples = len(audio_data.samples)

    # Create noise to add to the audio signal
    noise = np.random.normal(0, noise_level, num_samples).astype(np.float32)

    # Combine original audio with noise
    distorted_samples = audio_data.samples + noise

    # Ensure samples are within valid range
    distorted_samples = np.clip(distorted_samples, -1.0, 1.0)

    # Convert distorted samples back to int16 format for WAV writing
    distorted_samples_int16 = (distorted_samples * 32767).astype(np.int16)
    distorted_sound = miniaudio.DecodedSoundFile('music_title', audio_data.nchannels, audio_data.sample_rate, miniaudio.SampleFormat.SIGNED16, distorted_samples_int16)
    miniaudio.wav_write_file(output_file, distorted_sound)

add_vhs_distortion("test01.wav", "dist09.wav", 0.001);

# Example usage
# if __name__ == "__main__":
#     n = len(sys.argv)
#     src_wav = sys.argv[1];
#     dest_wav = sys.argv[2];
#     _noise_level = float(sys.argv[3]);
#     print(f"src wav:{src_wav}, dest_wav:{dest_wav}, noise_level:{_noise_level}");
#     print("Total arguments passed:", n)
#     add_vhs_distortion(src_wav, dest_wav, _noise_level);