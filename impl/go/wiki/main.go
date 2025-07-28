package main
import "fmt"


const (
  msg = "Hello, World!"
  key = "Secret"
)

func main() {
  var z state
  z.seed(key)
  fmt.Printf("Message: %s\nKey: %s\nXOR: %s\n", msg, key, z.vernam(msg))
}

type state struct {
  aa, bb, cc uint32
  mm [256]uint32
  randrsl  [256]uint32 // random number generator
  randcnt int // random number counter
}

func (z *state) isaac() {
  z.cc++
  z.bb += z.cc
  for i, x := range z.mm {
    switch i % 4 {
    case 0:
      z.aa = z.aa ^ z.aa << 13
    case 1:
      z.aa = z.aa ^ z.aa >> 6
    case 2:
      z.aa = z.aa ^ z.aa << 2
    case 3:
      z.aa = z.aa ^ z.aa >> 16
    }
    z.aa += z.mm[(i + 128) % 256]
    y := z.mm[x >> 2 % 256] + z.aa + z.bb
    z.mm[i] = y
    z.bb = z.mm[y >> 10 % 256] + x
    z.randrsl[i] = z.bb
  }
}

func (z *state) randInit() {
  const g = uint32(0x9e3779b9) // golden ratio
  a := [8]uint32{g, g, g, g, g, g, g, g}
  mixOne := func(i int, v uint32) {
    a[i] ^= v
    a[(i + 3) % 8] += a[i]
    a[(i + 1) % 8] += a[(i + 2) % 8]
  }
   
  mix := func() {
    mixOne(0, a[1] << 11)
    mixOne(0, a[2] >> 2)
    mixOne(0, a[3] << 8)
    mixOne(0, a[4] >> 16)
    mixOne(0, a[5] << 10)
    mixOne(0, a[6] >> 4)
    mixOne(0, a[7] << 8)
    mixOne(0, a[0] >> 9)
  }
  for i := 0; i < 256; i++ {
    mix()
  }

  for i := 0; i < 256; i += 8 {
    for j, rj := range z.randrsl[i : i + 8] {
      a[j] += rj
    }
  }
  for i := 0; i < 256; i += 8 {
    for j, mj := range z.mm[i : i + 8] {
      a[j] += mj
    }
    mix()
    for j, aj := range a {
      z.mm[i + j] = aj
    }
  }
  z.isaac()
}

func (z *state) seed(seed string) {
  for i, r := range seed {
    if i == 256 {
      break
    }
    z.randrsl[i] = uint32(r)
  }
  z.randInit()
}

func (z *state) random() (r uint32) {
  r = z.randrsl[z.randcnt]
  z.randcnt++
  if z.randcnt == 256 {
    z.isaac()
    z.randcnt = 0
  }
  return
}
func (z *state) randA() byte {
  return byte(z.random() % 95 + 32)
}

func (z *state) vernam(msg string) string {
  b := []byte(msg)
  for i := range b {
    b[i] ^= z.randA()
  }
  return fmt.Sprintf("%X", b)
}
