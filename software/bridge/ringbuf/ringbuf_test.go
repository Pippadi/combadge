package ringbuf

import "testing"

func TestRingBuf(t *testing.T) {
	rb := New(10)
	rb.AddWriter(0)
	for i := 0; i < 10; i++ {
		rb.Write(0, int64(i))
	}
	for i := 0; i < 10; i++ {
		if rb.Read() != int64(i) {
			t.Errorf("Read error")
		}
	}
}
