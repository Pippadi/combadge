package listener

import (
	"fmt"
	"net"

	"github.com/Pippadi/combadge/combridge/badge"
	"github.com/Pippadi/loggo"
	actor "gitlab.com/prithvivishak/goactor"
)

type Listener struct {
	actor.Base

	port        int
	tcpListener net.Listener
}

func New(p int) *Listener {
	return &Listener{port: p}
}

func (l *Listener) Initialize() (err error) {
	l.tcpListener, err = net.Listen("tcp", fmt.Sprintf("0.0.0.0:%d", l.port))
	if err != nil {
		return
	}

	go l.conditionallyAcceptConns()
	return nil
}

func (l *Listener) Finalize() {
	l.tcpListener.Close()
}

func (l *Listener) conditionallyAcceptConns() {
	for {
		incomingConn, err := l.tcpListener.Accept()
		if err != nil {
			loggo.Error(err)
			continue
		}

		// Unconditionally accept connections for now
		sendAcceptedBadge(l.CreatorInbox(), badge.New(incomingConn))
	}
}
