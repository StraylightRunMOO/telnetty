package telnetty

import (
	"errors"
	"sync"
	"sync/atomic"
)

var (
	errBufferFull  = errors.New("ring buffer full")
	errBufferEmpty = errors.New("ring buffer empty")
)

// pool for byte slices used by ring buffers and temporary buffers.
var (
	bufPool4k  = sync.Pool{New: func() interface{} { b := make([]byte, 4096); return &b }}
	bufPool16k = sync.Pool{New: func() interface{} { b := make([]byte, 16384); return &b }}
	bufPool64k = sync.Pool{New: func() interface{} { b := make([]byte, 65536); return &b }}
)

func getBuf(size int) []byte {
	switch {
	case size <= 4096:
		if bp := bufPool4k.Get().(*[]byte); cap(*bp) >= size {
			b := (*bp)[:size]
			return b
		}
	case size <= 16384:
		if bp := bufPool16k.Get().(*[]byte); cap(*bp) >= size {
			b := (*bp)[:size]
			return b
		}
	case size <= 65536:
		if bp := bufPool64k.Get().(*[]byte); cap(*bp) >= size {
			b := (*bp)[:size]
			return b
		}
	}
	return make([]byte, size)
}

func putBuf(b []byte) {
	c := cap(b)
	switch {
	case c == 4096:
		bufPool4k.Put(&b)
	case c == 16384:
		bufPool16k.Put(&b)
	case c == 65536:
		bufPool64k.Put(&b)
	}
}

// Ring is a lock-free single-producer single-consumer ring buffer.
// It uses power-of-two sizing with atomic indices for cache-friendly
// operation. Safe for one writer and one reader concurrently.
type Ring struct {
	buf   []byte
	mask  uint32
	_     [12]byte // padding to separate cache lines
	rhead uint32   // read head (consumer)
	_     [12]byte
	wtail uint32   // write tail (producer)
}

// NewRing creates a ring buffer with capacity rounded up to the next
// power of two. The usable capacity is size-1 to distinguish empty from full.
func NewRing(size int) *Ring {
	if size <= 0 {
		size = 4096
	}
	// Round up to power of two
	size--
	size |= size >> 1
	size |= size >> 2
	size |= size >> 4
	size |= size >> 8
	size |= size >> 16
	size++
	return &Ring{
		buf:  getBuf(size),
		mask: uint32(size - 1),
	}
}

// Free returns the ring buffer's underlying memory to the pool.
func (r *Ring) Free() {
	if r != nil && r.buf != nil {
		putBuf(r.buf)
		r.buf = nil
	}
}

// Len returns the number of bytes available to read.
func (r *Ring) Len() int {
	return int(atomic.LoadUint32(&r.wtail) - atomic.LoadUint32(&r.rhead))
}

// Cap returns the total capacity of the ring buffer.
func (r *Ring) Cap() int {
	return int(r.mask)
}

// Write copies p into the ring buffer. Returns the number of bytes written,
// which may be less than len(p) if the buffer is full.
func (r *Ring) Write(p []byte) (n int, err error) {
	w := atomic.LoadUint32(&r.wtail)
	rh := atomic.LoadUint32(&r.rhead)
	avail := int(r.mask) - int(w-rh)
	if avail <= 0 {
		return 0, errBufferFull
	}
	truncated := len(p) > avail
	if truncated {
		p = p[:avail]
	}
	for i, b := range p {
		r.buf[(w+uint32(i))&r.mask] = b
	}
	atomic.StoreUint32(&r.wtail, w+uint32(len(p)))
	if truncated {
		return len(p), errBufferFull
	}
	return len(p), nil
}

// Read copies up to len(p) bytes from the ring into p.
func (r *Ring) Read(p []byte) (n int, err error) {
	rh := atomic.LoadUint32(&r.rhead)
	w := atomic.LoadUint32(&r.wtail)
	avail := int(w - rh)
	if avail == 0 {
		return 0, errBufferEmpty
	}
	if len(p) > avail {
		p = p[:avail]
	}
	for i := range p {
		p[i] = r.buf[(rh+uint32(i))&r.mask]
	}
	atomic.StoreUint32(&r.rhead, rh+uint32(len(p)))
	return len(p), nil
}

// WriteByte writes a single byte. Returns errBufferFull if no space.
func (r *Ring) WriteByte(c byte) error {
	w := atomic.LoadUint32(&r.wtail)
	rh := atomic.LoadUint32(&r.rhead)
	if int(w-rh) >= int(r.mask) {
		return errBufferFull
	}
	r.buf[w&r.mask] = c
	atomic.StoreUint32(&r.wtail, w+1)
	return nil
}

// ReadByte reads a single byte. Returns errBufferEmpty if empty.
func (r *Ring) ReadByte() (byte, error) {
	rh := atomic.LoadUint32(&r.rhead)
	w := atomic.LoadUint32(&r.wtail)
	if w == rh {
		return 0, errBufferEmpty
	}
	b := r.buf[rh&r.mask]
	atomic.StoreUint32(&r.rhead, rh+1)
	return b, nil
}

// Peek returns a slice of up to n bytes available for reading without
// consuming them. The slice is only valid until the next Read/Write.
func (r *Ring) Peek(n int) []byte {
	rh := atomic.LoadUint32(&r.rhead)
	w := atomic.LoadUint32(&r.wtail)
	avail := int(w - rh)
	if n > avail {
		n = avail
	}
	if n == 0 {
		return nil
	}
	start := rh & r.mask
	end := start + uint32(n)
	if end <= uint32(len(r.buf)) {
		return r.buf[start:end]
	}
	// Wrap-around: caller must handle; we return contiguous portion
	return r.buf[start:uint32(len(r.buf))]
}

// Discard removes n bytes from the read side.
func (r *Ring) Discard(n int) {
	if n <= 0 {
		return
	}
	rh := atomic.LoadUint32(&r.rhead)
	w := atomic.LoadUint32(&r.wtail)
	avail := int(w - rh)
	if n > avail {
		n = avail
	}
	atomic.StoreUint32(&r.rhead, rh+uint32(n))
}

// Reset clears the buffer, resetting both indices to zero.
func (r *Ring) Reset() {
	atomic.StoreUint32(&r.rhead, 0)
	atomic.StoreUint32(&r.wtail, 0)
}

// Bytes returns all readable data as a newly allocated slice.
// This is the only allocating method in the hot path; use sparingly.
func (r *Ring) Bytes() []byte {
	l := r.Len()
	if l == 0 {
		return nil
	}
	p := make([]byte, l)
	rh := atomic.LoadUint32(&r.rhead)
	for i := 0; i < l; i++ {
		p[i] = r.buf[(rh+uint32(i))&r.mask]
	}
	return p
}

// chunkWriter wraps a Ring to implement io.Writer with chunking.
type chunkWriter struct {
	r *Ring
}

func (cw *chunkWriter) Write(p []byte) (int, error) {
	return cw.r.Write(p)
}

// StaticBuffer is a pre-allocated buffer that never grows beyond its cap.
// It avoids all allocations and is ideal for output buffering.
type StaticBuffer struct {
	buf   []byte
	limit int
}

// NewStaticBuffer creates a static buffer backed by a pooled slice.
func NewStaticBuffer(size int) *StaticBuffer {
	return &StaticBuffer{buf: getBuf(size)[:0], limit: size}
}

// Free returns the buffer to the pool.
func (sb *StaticBuffer) Free() {
	if sb != nil && sb.buf != nil {
		putBuf(sb.buf)
		sb.buf = nil
	}
}

// Write implements io.Writer.
func (sb *StaticBuffer) Write(p []byte) (int, error) {
	limit := sb.limit
	if limit == 0 {
		limit = cap(sb.buf)
	}
	avail := limit - len(sb.buf)
	if avail <= 0 {
		return 0, nil
	}
	if len(p) > avail {
		p = p[:avail]
	}
	sb.buf = append(sb.buf, p...)
	return len(p), nil
}

// WriteByte writes a single byte.
func (sb *StaticBuffer) WriteByte(c byte) error {
	limit := sb.limit
	if limit == 0 {
		limit = cap(sb.buf)
	}
	if len(sb.buf) >= limit {
		return errBufferFull
	}
	sb.buf = append(sb.buf, c)
	return nil
}

// Len returns the number of buffered bytes.
func (sb *StaticBuffer) Len() int { return len(sb.buf) }

// Cap returns the capacity.
func (sb *StaticBuffer) Cap() int {
	if sb.limit > 0 {
		return sb.limit
	}
	return cap(sb.buf)
}

// Bytes returns the buffered data. The slice is valid until the next Write.
func (sb *StaticBuffer) Bytes() []byte { return sb.buf }

// Reset clears the buffer without freeing memory.
func (sb *StaticBuffer) Reset() { sb.buf = sb.buf[:0] }

// Advance moves the read offset forward (for consuming data).
func (sb *StaticBuffer) Advance(n int) {
	if n >= len(sb.buf) {
		sb.buf = sb.buf[:0]
		return
	}
	copy(sb.buf, sb.buf[n:])
	sb.buf = sb.buf[:len(sb.buf)-n]
}
