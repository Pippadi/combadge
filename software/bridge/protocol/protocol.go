package protocol

import (
	"math"
	"unsafe"
)

type PacketType uint32

const (
	AudioStart PacketType = 0x01
	AudioStop             = 0x02
	AudioData             = 0x03
)

type Sample int16

const (
	BufLenSamples = 512
	SampleRate    = 44100

	SampleMax       = math.MaxInt16
	SampleMin       = math.MinInt16
	SampleSizeBytes = int(unsafe.Sizeof(Sample(0)))
)

type PacketHeader struct {
	Type PacketType
	Size uint32
}

type AudioBuf []Sample
