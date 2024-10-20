package ringbuf

import "testing"

func TestRingBuf(t *testing.T) {
	rb := New(10)
	rb.AddWriter(0)
	for i := 0; i < 10; i++ {
		rb.Write(0, int64(i))
	}
	for i := 0; i < 10; i++ {
		val, err := rb.Read()
		if err != nil {
			t.Errorf("Read error: %v", err)
		}
		if val != int64(i) {
			t.Errorf("Read error: expected %v, got %v", i, val)
		}
	}

	rb.AddWriter(1)
	rb.Write(1, 10)
	rb.WriteSlice(0, []int64{20, 2, 3, 4, 5})

	if rb.Available() != 5 {
		t.Errorf("Available error: expected 5, got %v", rb.Available())
	}

	expected := []int64{30, 2, 3, 4, 5}
	buf := make([]int64, len(expected))
	err := rb.ReadSlice(buf)
	if err != nil {
		t.Errorf("ReadSlice error: %v", err)
	}

	for i := 0; i < len(expected); i++ {
		if buf[i] != expected[i] {
			t.Errorf("ReadSlice error: expected %v, got %v", expected[i], buf[i])
		}
	}
}
