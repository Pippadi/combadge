package badge

import (
	"bytes"
	"encoding/binary"
	"io"
	"net"
	"unsafe"

	"github.com/Pippadi/combadge/combridge/protocol"
	"github.com/Pippadi/loggo"
	actor "gitlab.com/prithvivishak/goactor"
)

type Badge struct {
	actor.Base

	conn         net.Conn
	transmitting bool
}

func New(c net.Conn) *Badge {
	return &Badge{conn: c, transmitting: false}
}

func (b *Badge) Initialize() error {
	go b.listenForPackets()
	return nil
}

func (b *Badge) Finalize() {
	b.conn.Close()
	if b.transmitting {
		SendTransmitStop(b.CreatorInbox(), b.Inbox())
	}
}

func (b *Badge) Address() string {
	if b.conn == nil {
		return ""
	}
	return b.conn.RemoteAddr().String()
}

func (b *Badge) ProcessAudioFrom(src actor.Inbox, buf protocol.AudioBuf) {
	header := protocol.PacketHeader{
		Type: protocol.AudioData,
		Size: uint32(len(buf) * protocol.SampleSizeBytes),
	}
	binary.Write(b.conn, binary.LittleEndian, header)
	binary.Write(b.conn, binary.LittleEndian, buf)
}

func (b *Badge) RegisterTransmitStart(src actor.Inbox) {
	header := protocol.PacketHeader{
		Type: protocol.AudioStart,
		Size: 0,
	}
	binary.Write(b.conn, binary.LittleEndian, header)
}
func (b *Badge) RegisterTransmitStop(src actor.Inbox) {
	header := protocol.PacketHeader{
		Type: protocol.AudioStop,
		Size: 0,
	}
	binary.Write(b.conn, binary.LittleEndian, header)
}

func (b *Badge) listenForPackets() {
	defer actor.SendStopMsg(b.Inbox())
	for {
		header := protocol.PacketHeader{}
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
		case protocol.AudioStart:
			b.transmitting = true
			SendTransmitStart(b.CreatorInbox(), b.Inbox())
		case protocol.AudioStop:
			b.transmitting = false
			SendTransmitStop(b.CreatorInbox(), b.Inbox())
		case protocol.AudioData:
			rawBytes := make([]byte, int(header.Size))
			n, err := io.ReadFull(b.conn, rawBytes)
			if err != nil {
				return
			}
			var ab = make(protocol.AudioBuf, n/2)
			binary.Read(bytes.NewReader(rawBytes[:n]), binary.LittleEndian, ab)
			if err != nil {
				continue
			}
			SendAudioBuf(b.CreatorInbox(), b.Inbox(), ab)
		default:
			loggo.Infof("Unknown packet type 0x%x", header.Type)
		}
	}
}
