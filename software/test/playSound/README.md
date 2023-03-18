# playSound

A test to verify whether audio clip playback works with the MAX98357.

The sound clip header file was generated with [tools/soundClips](/software/tools/soundClips).
So far, I've had good results with 44100Hz sample rates for both the audio clip and the amplifier.

16000Hz playback works with the amplifier, but I just can't get a sound clip to play at that sample rate, and I don't know why.
I still need to test other sample rates (and verify that my process isn't at fault) to see where exactly it stops working.
