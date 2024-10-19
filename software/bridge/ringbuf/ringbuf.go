package ringbuf

import "fmt"

// RingBuf is a ring buffer implementation that supports multiple writers
type RingBuf struct {
	buf []int64

	// readPtr is the absolute position of the read pointer in the buffer
	readPtr int

	// writePtrs is a map of keys to write pointer positions, relative to the read pointer
	writePtrs []int
}

// New creates a new RingBuf with the given size
func New(size int) *RingBuf {
	return &RingBuf{
		buf:       make([]int64, size),
		readPtr:   0,
		writePtrs: make(map[any]int, 0),
	}
}

// AddWriter adds a new writer to the ring buffer with the given key
func (rb *RingBuf) AddWriter(key any) {
	rb.writePtrs[key] = readPtr
}

// RemoveWriter removes a writer from the ring buffer with the given key
func (rb *RingBuf) RemoveWriter(key any) {
	delete(rb.writePtrs, key)
}

// Write writes a value to the ring buffer for the given key
func (rb *RingBuf) Write(key any, val int64) error {
	ptr, ok := rb.writePtrs[key]
	if !ok {
		return fmt.Errorf("writer with key %v not found", key)
	}

	idx := absIdxFromRelative(ptr, rb.readPtr, len(rb.buf))

	rb.buf[idx] = val
	rb.writePtrs[key]++

	return nil
}

// Read reads a value from the ring buffer
func (rb *RingBuf) Read() int64 {
	val := rb.buf[rb.readPtr]
	rb.readPtr++
	for key, writePtr := range rb.writePtrs {
		if writePtr < 0 {
			rb.writePtrs[key] = 0
		} else {
			rb.writePtrs[key]++
		}
	}
	return val
}

// absIdxFromRelative converts a relative index to an absolute index
func absIdxFromRelative(idx int, readPtr int, bufLen int) int {
	absIdx := (readPtr + idx) % bufLen
	if absIdx < 0 {
		absIdx = bufLen + absIdx
	}

	return absIdx
}
