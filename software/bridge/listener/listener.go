package listener

import (
	"fmt"
	"net"

	"github.com/Pippadi/combadge/bridge/badge"
	"github.com/Pippadi/loggo"
	actor "gitlab.com/prithvivishak/goactor"
)

const port = 1701

type Listener struct {
	actor.Base

	tcpListener net.Listener
}

func (l *Listener) Initialize() (err error) {
	l.tcpListener, err = net.Listen("tcp", fmt.Sprintf("0.0.0.0:%d", port))
	if err != nil {
		return
	}

	go l.conditionallyAcceptConns()
	return nil
}

func (l *Listener) conditionallyAcceptConns() {
	defer l.tcpListener.Close()
	for {
		incomingConn, err := l.tcpListener.Accept()
		if err != nil {
			loggo.Error(err)
			continue
		}

		// Unconditionally accept connections for now
		incomingAddr := incomingConn.RemoteAddr().String()
		loggo.Info(incomingAddr, "connected")

		sendAcceptedBadge(l.CreatorInbox(), badge.New(incomingConn))
	}
}
