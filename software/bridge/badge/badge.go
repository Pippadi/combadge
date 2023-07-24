package badge

import (
	"bytes"
	"encoding/binary"
	"io"
	"net"
	"unsafe"

	"github.com/Pippadi/loggo"
	actor "gitlab.com/prithvivishak/goactor"
)

const (
	bufLenSamples  = 512
	bytesPerSample = 2

	audioStart = 0x01
	audioStop  = 0x02
	audioData  = 0x03
)

type packetHeader struct {
	Type uint32
	Size uint32
}

type AudioBuf [bufLenSamples]byte

type Badge struct {
	actor.Base

	conn net.Conn
}

func New(c net.Conn) *Badge {
	return &Badge{conn: c}
}

func (b *Badge) Initialize() error {
	go b.listenForPackets()
	return nil
}

func (b *Badge) Address() string {
	if b.conn == nil {
		return ""
	}
	return b.conn.LocalAddr().String()
}

func (b *Badge) listenForPackets() {
	defer actor.SendStopMsg(b.Inbox())
	defer b.conn.Close()
	for {
		header := packetHeader{}
		headerBytes := make([]byte, int(unsafe.Sizeof(header)))
		n, err := b.conn.Read(headerBytes)
		if err != nil {
			loggo.Info(err)
			return
		}
		if n != len(headerBytes) {
			continue
		}

		err = binary.Read(bytes.NewReader(headerBytes), binary.LittleEndian, &header)
		if err != nil {
			loggo.Error(err)
			continue
		}

		switch header.Type {
		case audioStart:
			loggo.Info(b.ID(), "starting audio")
		case audioStop:
			loggo.Info(b.ID(), "stopping audio")
		case audioData:
			loggo.Info(b.ID(), "sending audio")
			_, err = readAllFragments(b.conn, int(header.Size))
			if err != nil {
				return
			}
		default:
			loggo.Infof("Unknown packet type 0x%x", header.Type)
		}
	}
}

func readAllFragments(f io.Reader, size int) ([]byte, error) {
	final := make([]byte, 0)
	for len(final) != size {
		chunk := make([]byte, size-len(final))
		n, err := f.Read(chunk)
		if err != nil {
			return nil, err
		}
		final = append(final, chunk[:n]...)
	}
	return final, nil
}
