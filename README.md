# Zero Punctuator

Removes silent sections from videos. Useful to remove pauses from lectures.

## main.c

Detects silent sections in audio files.

* Input: audio.wav file in the same folder as the executable
* Output: ranges.rng file that contains silent ranges in the following format:
  1. 4 bytes - unsigned int - start time in ms
  2. 4 bytes - unsigned int - end time in ms
  3. GOTO 1
