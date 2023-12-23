# max98357TCPStream

A test for streaming audio data over TCP to the MAX98357.

Set WiFi credentials and I2S configurations as necessary in `max98357TCPStream.ino`.
Configure `streamSound.py` to use the corresponding settings as well.

Next, use `ffmpeg` to convert a given audio file to the required raw format. Change settings if necessary.

```sh
ffmpeg -y -i YourSound.mp3 -f s16le -ac 1 -ar 44100 YourSound.raw
```

Finally, upload the code to the ESP32, and invoke the python script.

```sh
python3 streamSound.py YourSound.raw
```

Audio playback should be fairly smooth. If you have noise, it is very likely to be on the power supply line.
Probing with an oscilloscope may be useful to diagnose issues. See [hardware/pcb](/hardware/pcb#crackling-on-speaker) for more insight.

To make sure it's not your testing scripts that are at fault, use `streamSound.py` in conjunction with [/software/test/inmp441TCPStream/tcpRecorder.py](/software/test/inmp441TCPStream/tcpRecorder.py) to make sure that the sound you put in is the same sound you get out.
