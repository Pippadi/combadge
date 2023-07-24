package badge

import actor "gitlab.com/prithvivishak/goactor"

type badgeManager interface {
	ProcessAudio(buf AudioBuf)
}

func sendAudioBuf(dest actor.Inbox, buf AudioBuf) {
	dest <- func(a actor.Actor) error {
		a.(badgeManager).ProcessAudio(buf)
		return nil
	}
}
