package bridge

import (
	"strings"
	"sync"
	"time"

	"github.com/Pippadi/combadge/combridge/badge"
	"github.com/Pippadi/combadge/combridge/listener"
	"github.com/Pippadi/combadge/combridge/protocol"
	"github.com/Pippadi/loggo"
	actor "gitlab.com/prithvivishak/goactor"
)

const (
	port = 1701

	bufFullInterval  = time.Second * protocol.BufLenSamples / protocol.SampleRate
	transmitInterval = 3 * bufFullInterval / 4
)

type streamInfo struct {
	LastTransmitted protocol.AudioBuf
	Transmitting    bool
	Receiving       bool
}

type Bridge struct {
	actor.Base

	transmittingCnt int

	// badges maps badge actors' inbox to their most recently sent packet
	badges      map[actor.Inbox]*streamInfo
	badgesMutex *sync.Mutex

	processingBuffer []int64
}

func New() *Bridge {
	return &Bridge{
		transmittingCnt:  0,
		badges:           make(map[actor.Inbox]*streamInfo),
		badgesMutex:      new(sync.Mutex),
		processingBuffer: make([]int64, protocol.BufLenSamples),
	}
}

func (b *Bridge) Initialize() error {
	_, err := b.SpawnNested(listener.New(port), "Listener")
	go b.periodicallySendBuffer()
	return err
}

func (b *Bridge) periodicallySendBuffer() {
	shouldTransmit := false
	wasReceiving := false
	for {
		wasReceiving = shouldTransmit
		time.Sleep(transmitInterval)

		shouldTransmit = b.shouldTransmit()
		if !shouldTransmit {
			if wasReceiving {
				for ibx, si := range b.badges {
					loggo.Debug("Ending transmission to", ibx)
					badge.SendTransmitStop(ibx, b.Inbox())
					si.Receiving = false
				}
			}
			continue
		}

		b.badgesMutex.Lock()
		for ibx, si := range b.badges {
			if si.Receiving == false && !(b.transmittingCnt == 1 && si.Transmitting) {
				loggo.Debug("Initiating transmission to", ibx)
				badge.SendTransmitStart(ibx, b.Inbox())
				si.Receiving = true
			}
		}

		b.sumBuffers()
		for ibx, si := range b.badges {
			if si.Receiving {
				var toTransmit = make(protocol.AudioBuf, protocol.BufLenSamples)
				if si.LastTransmitted != nil {
					stagingBuf := make([]int64, protocol.BufLenSamples)
					for i, val := range si.LastTransmitted {
						stagingBuf[i] = b.processingBuffer[i] - int64(val)
					}
					truncateToAudioBuf(toTransmit, stagingBuf)
				} else {
					truncateToAudioBuf(toTransmit, b.processingBuffer)
				}
				badge.SendAudioBuf(ibx, b.Inbox(), toTransmit)
				si.LastTransmitted = nil
				//loggo.Debug("Sending to", ibx)
			}
		}
		b.badgesMutex.Unlock()
	}
}

func (b *Bridge) shouldTransmit() bool {
	return b.transmittingCnt > 0 && len(b.badges) > 1
}

func (b *Bridge) sumBuffers() {
	for i, _ := range b.processingBuffer {
		b.processingBuffer[i] = 0
	}
	for _, si := range b.badges {
		if si.LastTransmitted != nil {
			for i, val := range si.LastTransmitted {
				b.processingBuffer[i] += int64(val)
			}
		}
	}
}

func (b *Bridge) AcceptBadge(bdg *badge.Badge) {
	_, err := b.SpawnNested(bdg, "Badge "+bdg.Address())
	if err != nil {
		loggo.Error(err)
		return
	}
	loggo.Info(bdg.ID(), "connected at", bdg.Inbox())
	b.badgesMutex.Lock()
	b.badges[bdg.Inbox()] = &streamInfo{Transmitting: false, Receiving: false}

	if b.shouldTransmit() {
		badge.SendTransmitStart(bdg.Inbox(), b.Inbox())
		b.badges[bdg.Inbox()].Receiving = true
	}
	b.badgesMutex.Unlock()
}

func (b *Bridge) HandleLastMsg(a actor.Actor, err error) error {
	if strings.HasPrefix(a.ID(), "Badge") {
		b.badgesMutex.Lock()
		delete(b.badges, a.Inbox())
		b.badgesMutex.Unlock()
		loggo.Info(a.ID(), "disconnected")
	}
	return nil
}

func (b *Bridge) RegisterTransmitStart(src actor.Inbox) {
	loggo.Debug(src, "started transmitting")
	b.transmittingCnt++
	b.badgesMutex.Lock()
	b.badges[src].Transmitting = true
	b.badgesMutex.Unlock()
}
func (b *Bridge) RegisterTransmitStop(src actor.Inbox) {
	loggo.Debug(src, "stopped transmitting")
	b.transmittingCnt--
	b.badgesMutex.Lock()
	b.badges[src].Transmitting = false
	b.badgesMutex.Unlock()
}

func (b *Bridge) ProcessAudioFrom(src actor.Inbox, dat protocol.AudioBuf) {
	b.badgesMutex.Lock()
	b.badges[src].LastTransmitted = dat
	b.badgesMutex.Unlock()
}

func truncateToAudioBuf(audioBuf protocol.AudioBuf, stagingBuf []int64) {
	for i, v := range stagingBuf {
		if v > protocol.SampleMax {
			audioBuf[i] = protocol.SampleMax
		} else if v < protocol.SampleMin {
			audioBuf[i] = protocol.SampleMin
		} else {
			audioBuf[i] = protocol.Sample(v)
		}
	}
}
