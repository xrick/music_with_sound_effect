from pedalboard import Pedalboard, Reverb, load_plugin
from pedalboard.io import AudioFile
from mido import Message # not part of Pedalboard, but convenient!
# import timeit
import psutil
import time
import cProfile
# from memory_profiler import profile
import tracemalloc

def apply_uvi_tape_flanger_effect(src_wav=None, output_wav=None):
    effect = load_plugin("../../../sound_effects/UVI Tape Flanger.vst3")
    # for e in effect.parameters.items():
    #     print(e)
    effect.ration = 15;
    effect.apply_tape_simulation = True
    effect.delay = '12.0 ms';
    with AudioFile(src_wav) as f:
  # Open an audio file to write to:
      with AudioFile(output_wav, 'w', f.samplerate, f.num_channels) as o:
         # Read one second of audio at a time, until the file is empty:
        while f.tell() < f.frames:
          chunk = f.read(f.samplerate)
          # Run the audio through our pedalboard:
          effected = effect(chunk, f.samplerate, reset=False)
          # Write the output to our output file:
          o.write(effected)

if __name__ == "__main__":
   apply_uvi_tape_flanger_effect('test02.wav','./output_wav/output_uvi_flanger.wav');