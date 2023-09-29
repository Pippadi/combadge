package badge

import (
	"github.com/Pippadi/combadge/combridge/protocol"
	actor "gitlab.com/prithvivishak/goactor"
)

type audioDev interface {
	ProcessAudioFrom(src actor.Inbox, buf protocol.AudioBuf)
	RegisterTransmitStart(src actor.Inbox)
	RegisterTransmitStop(src actor.Inbox)
}

func SendAudioBuf(dest actor.Inbox, src actor.Inbox, buf protocol.AudioBuf) {
	dest <- func(a actor.Actor) error {
		a.(audioDev).ProcessAudioFrom(src, buf)
		return nil
	}
}

func SendTransmitStart(dest actor.Inbox, src actor.Inbox) {
	dest <- func(a actor.Actor) error {
		a.(audioDev).RegisterTransmitStart(src)
		return nil
	}
}
func SendTransmitStop(dest actor.Inbox, src actor.Inbox) {
	dest <- func(a actor.Actor) error {
		a.(audioDev).RegisterTransmitStop(src)
		return nil
	}
}
