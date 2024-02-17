#ifndef crap_h
#define crap_h

#define AUDIO_START 0x01
#define AUDIO_STOP 0x02
#define AUDIO_DATA 0x03

typedef struct {
    uint32_t type; // No padding = good
    uint32_t size;
} PacketHeader;

typedef struct {
    PacketHeader header;
    sample_t data[BUF_LEN_SAMPLES];
} AudioPacket;

#endif
