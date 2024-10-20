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

	rb.AddWriter(1)
	rb.Write(1, 10)
	rb.WriteSlice(0, []int64{20, 2, 3, 4, 5})

	expected := []int64{30, 2, 3, 4, 5, 0, 0, 0, 0, 0}
	buf := make([]int64, 10)
	rb.ReadSlice(buf)

	for i := 0; i < 10; i++ {
		if buf[i] != expected[i] {
			t.Errorf("ReadSlice error: expected %v, got %v", expected[i], buf[i])
		}
	}
}
