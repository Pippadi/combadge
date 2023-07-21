package main

import (
	"fmt"
	"net"
	"os"

	"github.com/Pippadi/loggo"
)

const (
	port           = 1701
	bufLen         = 2048
	packetLen      = bufLen + 1
	bytesPerSample = 2

	audioStart = 0x01
	audioStop  = 0x02
	audioData  = 0x03
)

func main() {
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
		incomingAddr := incomingConn.RemoteAddr().String()
		go func() {
			buf := make([]byte, packetLen*bytesPerSample)
			for {
				n, err := incomingConn.Read(buf)
				if err != nil {
					return
				}
				if n > 0 {
					switch buf[0] {
					case audioStart:
						loggo.Info(incomingAddr, "starting audio")
					case audioStop:
						loggo.Info(incomingAddr, "stopping audio")
					case audioData:
						loggo.Info(incomingAddr, "sending audio")
					}
				}
			}
		}()
	}
}
