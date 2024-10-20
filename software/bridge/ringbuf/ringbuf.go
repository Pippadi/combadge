package ringbuf

import "fmt"

// RingBuf is a ring buffer implementation that supports multiple writers.
// Values written by each writer are summed in each cell of the buffer.
// Cells are zeroed when read.
type RingBuf struct {
	buf []int64

	// readPtr is the absolute position of the read pointer in the buffer
	readPtr int

	// writePtrs is a map of keys to write pointer positions, relative to the read pointer
	writePtrs map[any]int
}

// New creates a new RingBuf with the given size
func New(size int) *RingBuf {
	return &RingBuf{
		buf:       make([]int64, size),
		readPtr:   0,
		writePtrs: make(map[any]int),
	}
}

// AddWriter adds a new writer to the ring buffer with the given key
func (rb *RingBuf) AddWriter(key any) {
	rb.writePtrs[key] = rb.readPtr
}

// RemoveWriter removes a writer from the ring buffer with the given key
func (rb *RingBuf) RemoveWriter(key any) {
	delete(rb.writePtrs, key)
}

// Write writes a value to the ring buffer for the given key.
// The value is summed with the existing value in the buffer.
func (rb *RingBuf) Write(key any, val int64) error {
	ptr, ok := rb.writePtrs[key]
	if !ok {
		return fmt.Errorf("writer with key %v not found", key)
	}

	if ptr >= len(rb.buf) {
		return fmt.Errorf("writer with key %v has reached the end of the buffer", key)
	}

	idx := absIdxFromRelative(ptr, rb.readPtr, len(rb.buf))

	rb.buf[idx] += val
	rb.writePtrs[key]++

	return nil
}

// Read reads a value from the ring buffer.
// The value is zeroed in the buffer.
func (rb *RingBuf) Read() int64 {
	val := rb.buf[rb.readPtr]
	rb.buf[rb.readPtr] = 0

	rb.readPtr = (rb.readPtr + 1) % len(rb.buf)
	for key, writePtr := range rb.writePtrs {
		if writePtr < 0 {
			rb.writePtrs[key] = 0
		} else {
			rb.writePtrs[key]--
		}
	}

	return val
}

func (rb *RingBuf) WriteSlice(key any, vals []int64) error {
	for _, val := range vals {
		if err := rb.Write(key, val); err != nil {
			return err
		}
	}
	return nil
}

func (rb *RingBuf) ReadSlice(buf []int64) error {
	for i := 0; i < len(buf); i++ {
		buf[i] = rb.Read()
	}
	return nil
}

func (rb *RingBuf) String() string {
	return fmt.Sprintf("RingBuf{buf: %v, readPtr: %v, writePtrs: %v}", rb.buf, rb.readPtr, rb.writePtrs)
}

// absIdxFromRelative converts a relative index to an absolute index
func absIdxFromRelative(idx int, readPtr int, bufLen int) int {
	absIdx := (readPtr + idx) % bufLen
	if absIdx < 0 {
		absIdx = bufLen + absIdx
	}

	return absIdx
}
