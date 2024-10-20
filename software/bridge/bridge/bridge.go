package bridge

import (
	"strings"
	"sync"
	"time"

	"github.com/Pippadi/combadge/combridge/badge"
	"github.com/Pippadi/combadge/combridge/listener"
	"github.com/Pippadi/combadge/combridge/protocol"
	"github.com/Pippadi/combadge/combridge/ringbuf"
	"github.com/Pippadi/loggo"
	actor "gitlab.com/prithvivishak/goactor"
)

const (
	port = 1701

	bufFullInterval  = time.Second * protocol.BufLenSamples / protocol.SampleRate
	transmitInterval = bufFullInterval / 2

	bufSize = protocol.BufLenSamples * 4
)

type badgeState struct {
	Transmitting bool
	Receiving    bool
}

// Bridge is the main actor that manages the connections and streaming between badges
type Bridge struct {
	actor.Base

	badges      map[actor.Inbox]*badgeState
	badgesMutex *sync.Mutex

	// transmittingCnt is the number of badges currently transmitting.
	// We can't just iterate over the map because there may be situations where that causes panics
	transmittingCnt int

	processingBuffer *ringbuf.RingBuf
}

// New creates a new Bridge
func New() *Bridge {
	return &Bridge{
		transmittingCnt:  0,
		badges:           make(map[actor.Inbox]*badgeState),
		badgesMutex:      new(sync.Mutex),
		processingBuffer: ringbuf.New(bufSize),
	}
}

// Initialize starts the Bridge
func (b *Bridge) Initialize() error {
	_, err := b.SpawnNested(listener.New(port), "Listener")
	go b.periodicallySendBuffer()
	return err
}

// periodicallySendBuffer sends the buffer to all connected badges at a regular interval
func (b *Bridge) periodicallySendBuffer() {
	shouldTransmit := false
	for {
		time.Sleep(transmitInterval)

		shouldTransmit = b.shouldTransmit()
		if !shouldTransmit {
			for ibx, bs := range b.badges {
				if bs.Receiving {
					loggo.Debug("Ending transmission to", ibx)
					badge.SendTransmitStop(ibx, b.Inbox())
					bs.Receiving = false
				}
			}
			continue
		}

		b.badgesMutex.Lock()
		for ibx, bs := range b.badges {
			if bs.Receiving == false && !(b.transmittingCnt == 1 && bs.Transmitting) {
				loggo.Debug("Initiating transmission to", ibx)
				badge.SendTransmitStart(ibx, b.Inbox())
				bs.Receiving = true
			}
		}

		outBufLen := b.processingBuffer.Available()
		if protocol.BufLenSamples < outBufLen {
			outBufLen = protocol.BufLenSamples
		}

		outBufI64 := make([]int64, outBufLen)
		err := b.processingBuffer.ReadSlice(outBufI64)
		if err != nil {
			loggo.Error(err)
			b.badgesMutex.Unlock()
			continue
		}

		outBuf := make(protocol.AudioBuf, outBufLen)
		truncateToAudioBuf(outBuf, outBufI64)

		for ibx, bs := range b.badges {
			if bs.Receiving {
				badge.SendAudioBuf(ibx, b.Inbox(), outBuf)
				//loggo.Debug("Sending to", ibx)
			}
		}
		b.badgesMutex.Unlock()
	}
}

// shouldTransmit returns true if there are more than one badges connected and at least one is transmitting.
func (b *Bridge) shouldTransmit() bool {
	return b.transmittingCnt > 0 && len(b.badges) > 1
}

// AcceptBadge adds a badge to the Bridge.
func (b *Bridge) AcceptBadge(bdg *badge.Badge) {
	_, err := b.SpawnNested(bdg, "Badge "+bdg.Address())
	if err != nil {
		loggo.Error(err)
		return
	}

	loggo.Info(bdg.ID(), "connected at", bdg.Inbox())
	b.badgesMutex.Lock()
	b.badges[bdg.Inbox()] = &badgeState{Transmitting: false, Receiving: false}
	b.processingBuffer.AddWriter(bdg.Inbox())

	if b.shouldTransmit() {
		badge.SendTransmitStart(bdg.Inbox(), b.Inbox())
		b.badges[bdg.Inbox()].Receiving = true
	}
	b.badgesMutex.Unlock()
}

// HandleLastMsg removes a badge from the Bridge.
func (b *Bridge) HandleLastMsg(a actor.Actor, err error) error {
	if strings.HasPrefix(a.ID(), "Badge") {
		b.badgesMutex.Lock()
		delete(b.badges, a.Inbox())
		b.badgesMutex.Unlock()

		b.processingBuffer.RemoveWriter(a.Inbox())
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
	datI64 := make([]int64, len(dat))
	for i, v := range dat {
		datI64[i] = int64(v)
	}
	b.processingBuffer.WriteSlice(src, datI64)
}

// truncateToAudioBuf truncates the samples of the staging buffer to the range of a protocol.AudioBuf
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
