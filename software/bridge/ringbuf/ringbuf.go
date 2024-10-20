package ringbuf

import "fmt"

// RingBuf is a ring buffer implementation that supports multiple writers.
// Values written by each writer are summed in each cell of the buffer.
// Cells are zeroed when read.
type RingBuf struct {
	buf []int64

	// readPtr is the absolute position of the read pointer in the buffer.
	readPtr int

	// writePtrs is a map of keys to write pointer offsets, relative to the read pointer.
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

// AddWriter adds a new writer to the ring buffer with the given key.
func (rb *RingBuf) AddWriter(key any) {
	rb.writePtrs[key] = rb.readPtr
}

// RemoveWriter removes a writer from the ring buffer with the given key.
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

	idx := bufIdxFromOffset(ptr, rb.readPtr, len(rb.buf))

	rb.buf[idx] += val
	rb.writePtrs[key]++

	return nil
}

// Read reads a value from the ring buffer.
// The value is zeroed in the buffer.
func (rb *RingBuf) Read() (int64, error) {
	if rb.Available() == 0 {
		return 0, fmt.Errorf("no values available to read")
	}

	val := rb.buf[rb.readPtr]
	rb.buf[rb.readPtr] = 0

	rb.readPtr = bufIdxFromOffset(1, rb.readPtr, len(rb.buf))
	for key, writePtr := range rb.writePtrs {
		if writePtr > 0 {
			rb.writePtrs[key]--
		}
		// If writePtr == 0, the writer has reached the read pointer.
		// We don't want write pointers behind the read pointers, so they're carried forward.
	}

	return val, nil
}

// WriteSlice writes a slice of values to the ring buffer for the given key.
func (rb *RingBuf) WriteSlice(key any, vals []int64) error {
	for _, val := range vals {
		if err := rb.Write(key, val); err != nil {
			return err
		}
	}
	return nil
}

// ReadSlice reads a slice of values from the ring buffer.
func (rb *RingBuf) ReadSlice(buf []int64) error {
	var err error = nil
	for i := 0; i < len(buf); i++ {
		buf[i], err = rb.Read()
		if err != nil {
			break
		}
	}
	return err
}

// Available returns the distance between the read pointer and the write pointer farthest ahead.
func (rb *RingBuf) Available() int {
	farthestPtr := 0
	for _, ptr := range rb.writePtrs {
		if ptr > farthestPtr {
			farthestPtr = ptr
		}
	}

	return farthestPtr
}

func (rb *RingBuf) String() string {
	return fmt.Sprintf("RingBuf{buf: %v, readPtr: %v, writePtrs: %v}", rb.buf, rb.readPtr, rb.writePtrs)
}

// bufIdxFromOffset converts an offset to a buffer index.
func bufIdxFromOffset(offset int, readPtr int, bufLen int) int {
	return (readPtr + offset) % bufLen
}
