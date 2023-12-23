package main

import (
	"os"
	"os/signal"
	"syscall"

	"github.com/Pippadi/combadge/combridge/bridge"
	"github.com/Pippadi/loggo"
	actor "gitlab.com/prithvivishak/goactor"
)

func main() {
	loggo.SetLevel(loggo.DebugLevel)
	stop, bridgeIbx, err := actor.SpawnRoot(bridge.New(), "Bridge")
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
