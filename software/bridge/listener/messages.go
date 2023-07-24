package listener

import (
	"github.com/Pippadi/combadge/combridge/badge"
	actor "gitlab.com/prithvivishak/goactor"
)

type badgeReceiver interface {
	AcceptBadge(b *badge.Badge)
}

func sendAcceptedBadge(dest actor.Inbox, b *badge.Badge) {
	dest <- func(a actor.Actor) error {
		a.(badgeReceiver).AcceptBadge(b)
		return nil
	}
}
