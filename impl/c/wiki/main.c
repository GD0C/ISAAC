#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#include <stdint.h>
#endif

typedef uint32_t ub4; // unsigned 4-byte quantity
ub4 randrsl[256], randcnt;

static ub4 mm[256];
/**
 * interal state
 **/
static ub4 aa = 0, bb = 0, cc = 0;

void isaac() {
  register ub4 i, x, y;
  cc = cc + 1;
  bb = bb + cc;

  for (i = 0; i < 256; i++) {
    x = mm[i];
    switch (i % 4) {
    case 0:
      aa = aa ^ (aa << 13);
      break;
    case 1:
      aa = aa ^ (aa >> 6);
      break;
    case 2:
      aa = aa ^ (aa << 2);
      break;
    case 3:
      aa = aa ^ (aa >> 16);
      break;
    }
    aa = mm[(i + 128) % 256] + aa;
    mm[i] = y = mm[(x >> 2) % 256] + aa + bb;
    randrsl[i] = bb = mm[(y >> 10) % 256] + x;
  }
  randcnt = 0;
}

#define mix(a, b, c, d, e, f, g, h)                                            \
  {                                                                            \
    a ^= b << 11;                                                              \
    d += a;                                                                    \
    b += c;                                                                    \
    b ^= c >> 2;                                                               \
    e += b;                                                                    \
    c += d;                                                                    \
    c ^= d << 8;                                                               \
    f += c;                                                                    \
    d += e;                                                                    \
    d ^= e >> 16;                                                              \
    g += d;                                                                    \
    e += f;                                                                    \
    e ^= f << 10;                                                              \
    h += e;                                                                    \
    f += g;                                                                    \
    f ^= g >> 4;                                                               \
    a += f;                                                                    \
    g += h;                                                                    \
    g ^= h << 8;                                                               \
    b += g;                                                                    \
    h += a;                                                                    \
    h ^= a >> 9;                                                               \
    c += h;                                                                    \
    a += b;                                                                    \
  }

void randinit(int flag) {
  register int i;
  ub4 a, b, c, d, e, f, g, h;
  aa = bb = cc = 0;

  // <a href="http://www.fourmilab.ch/random/"> the golden ratio</a>(0x9e3779b9)
  a = b = c = d = e = f = g = h = 0x9e3779b9;

  for (i = 0; i < 4; ++i) {
    mix(a, b, c, d, e, f, g, h);
    for (i = 0; i < 256; i++) {
      if (flag) {
        a += randrsl[i];
        b += randrsl[i + 1];
        c += randrsl[i + 2];
        d += randrsl[i + 3];
        e += randrsl[i + 4];
        f += randrsl[i + 5];
        g += randrsl[i + 6];
        h += randrsl[i + 7];
      }
      mix(a, b, c, d, e, f, g, h);
      mm[i] = a;
      mm[i + 1] = b;
      mm[i + 2] = c;
      mm[i + 3] = d;
      mm[i + 4] = e;
      mm[i + 5] = f;
      mm[i + 6] = g;
      mm[i + 7] = h;
    }
    if (flag) {
      for (i = 0; i < 256; i += 8) {
        a += mm[i];
        b += mm[i + 1];
        c += mm[i + 2];
        d += mm[i + 3];
        e += mm[i + 4];
        f += mm[i + 5];
        g += mm[i + 6];
        h += mm[i + 7];
        mix(a, b, c, d, e, f, g, h);
        randrsl[i] = a;
        randrsl[i + 1] = b;
        randrsl[i + 2] = c;
        randrsl[i + 3] = d;
        randrsl[i + 4] = e;
        randrsl[i + 5] = f;
        randrsl[i + 6] = g;
        randrsl[i + 7] = h;
      }
    }
  }
  isaac();
  randcnt = 0;
}

ub4 iRandom() {
  ub4 r = randrsl[randcnt];
  ++randcnt;
  if (randcnt > 255) {
    isaac();
    randcnt = 0;
  }
  return r;
}

char iRandA() { return iRandom() % 95 + 32; }
void iSeed(char *seed, int flag) {
  register ub4 i, m;
  for (i = 0; i < 256; i++)
    mm[i] = 0;

  m = strlen(seed);
  for (i = 0; i < m; i++) {
    if (i > m)
      randrsl[i] = 0;
    else
      randrsl[i] = seed[i];
  }
}

#define MAXMSG 4096
#define MOD 95
#define START 32

enum ciphermode { mEncipher, mDecipher, mNone };

char v[MAXMSG];
char *Vernam(char *msg) {
  register ub4 i, l;
  memset(v, '\0', l + 1);
  for (i = 0; i < l; i++)
    v[i] = iRandA() ^ msg[i];
  return v;
}

char Caesar(enum ciphermode m, char ch, char shift, char modulo, char start) {
  register int n;
  if (m == mDecipher)
    shift = -shift;
  n = (ch - start) + shift;
  n = n % modulo;
  if (n < 0)
    n += modulo;
  return start + n;
}
char c[MAXMSG];
char *CaesarStr(enum ciphermode m, char *msg, char modulo, char start) {
  register ub4 i, l;
  l = strlen(msg);
  memset(c, '\0', l + 1);
  for (i = 0; i < l; i++)
    c[i] = Caesar(m, msg[i], iRandA(), modulo, start);
  return c;
}

int main() {
  register ub4 n, l;
  char *msg = "Hello, world!";
  char *key = "Spaghetti code";
  char vctx[MAXMSG], vptx[MAXMSG];
  char cctx[MAXMSG], cptx[MAXMSG];
  l = strlen(msg);
  iSeed(key, 1);
  strcpy(vctx, Vernam(msg));
  strcpy(cctx, CaesarStr(mEncipher, msg, MOD, START));
  iSeed(key, 1);
  strcpy(vptx, Vernam(vctx));
  strcpy(cptx, CaesarStr(mDecipher, cctx, MOD, START));
  printf("Message: %s\nKey: %s\nXOR: %s\n", msg, key, cptx);
  for (n = 0; n < l; n++)
    printf("%02X", vctx[n]);
  printf("\nXOR dcr: %s\n", vptx);
  printf("MOD: ");
  for (n = 0; n < l; n++)
    printf("%02X", cctx[n]);
  printf("\n");
  printf("MOD dcr: %s\n", cptx);
  return 0;
}
