package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"net"
	"os"
	"unsafe"

	"github.com/Pippadi/loggo"
)

const (
	port           = 1701
	bufLenSamples  = 512
	bytesPerSample = 2
	packetLenBytes = bufLenSamples*bytesPerSample + 1

	audioStart = 0x01
	audioStop  = 0x02
	audioData  = 0x03
)

type packetHeader struct {
	Type uint32
	Size uint32
}

type audioPacket struct {
	Header packetHeader
	Data   [bufLenSamples * bytesPerSample]byte
}

func main() {
	loggo.SetLevel(loggo.DebugLevel)
	listener, err := net.Listen("tcp", fmt.Sprintf("0.0.0.0:%d", port))
	if err != nil {
		loggo.Error(err)
		os.Exit(1)
	}

	for {
		incomingConn, err := listener.Accept()
		if err != nil {
			loggo.Error(err)
			continue
		}

		go func() {
			incomingAddr := incomingConn.RemoteAddr().String()
			loggo.Info(incomingAddr, "connected")
			//reader := bufio.NewReader(incomingConn)
			for {
				header := packetHeader{}
				headerBytes := make([]byte, int(unsafe.Sizeof(header)))
				n, err := incomingConn.Read(headerBytes)
				if err != nil {
					loggo.Error(err)
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
					loggo.Info(incomingAddr, "starting audio")
				case audioStop:
					loggo.Info(incomingAddr, "stopping audio")
				case audioData:
					loggo.Info(incomingAddr, "sending audio")
					_, err = readAllFragments(incomingConn, int(header.Size))
					if err != nil {
						return
					}
				default:
					loggo.Infof("Unknown packet type 0x%x", header.Type)
				}
			}
		}()
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
