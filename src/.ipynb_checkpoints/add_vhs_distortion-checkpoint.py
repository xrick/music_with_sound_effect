import miniaudio
import numpy as np

def add_vhs_distortion(input_file, output_file, noise_lel):
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

# Example usage
add_vhs_distortion("test01.wav", "output_distorted.wav")