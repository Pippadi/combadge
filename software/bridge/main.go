package main

import (
	"os"
	"os/signal"
	"syscall"

	"github.com/Pippadi/combadge/bridge/badge"
	"github.com/Pippadi/combadge/bridge/listener"
	"github.com/Pippadi/loggo"
	actor "gitlab.com/prithvivishak/goactor"
)

type bridge struct {
	actor.Base
}

func (b *bridge) Initialize() error {
	_, err := b.SpawnNested(new(listener.Listener), "Listener")
	return err
}

func (b *bridge) AcceptBadge(bdg *badge.Badge) {
	loggo.Info("Badge connected")
}

func (b *bridge) ProcessAudio(dat badge.AudioBuf) {
	loggo.Info("Got data")
}

func main() {
	stop, bridgeIbx, err := actor.SpawnRoot(new(bridge), "Bridge")
	if err != nil {
		loggo.Error(err)
		os.Exit(1)
	}

	terminator := make(chan os.Signal)
	signal.Notify(terminator, syscall.SIGTERM, syscall.SIGINT)

	select {
	case <-stop:
		return
	case <-terminator:
		actor.SendStopMsg(bridgeIbx)
	}
}
