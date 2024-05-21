# ifndef __CACHE_H
#   define __CACHE_H

#   define __CACHE_VERSION_MAJOR 0
#   define __CACHE_VERSION_MINOR 0
#   define __CACHE_VERSION_PATCH 0
#   define __CACHE_VERSION            \
    (                                 \
      (__CACHE_VERSION_MAJOR << 24) | \
      (__CACHE_VERSION_MINOR << 16) | \
      (__CACHE_VERSION_PATCH <<  0)   \
    )

#   define _In
#   define _Out
#   define _InOut

#   define u_round_up(x, y)   ((x) / (y) + ((x) % (y) != 0))
#   define u_round_down(x, y) ((x) / (y))

#   include <inttypes.h>
#   include <stdint.h>
#   include <stdio.h>

typedef uint8_t  u_byte_t;
typedef uint16_t u_half_t;
typedef uint32_t u_word_t;
typedef uint64_t u_long_t;

#   define U_BYTE(n) UINT8_C(n)
#   define U_HALF(n) UINT16_C(n)
#   define U_WORD(n) UINT32_C(n)
#   define U_LONG(n) UINT64_C(n)

#   define U_BYTE_MAX UINT8_MAX
#   define U_HALF_MAX UINT16_MAX
#   define U_WORD_MAX UINT32_MAX
#   define U_LONG_MAX UINT64_MAX

#   define U_BYTE_FMTD PRIu8
#   define U_HALF_FMTD PRIu16
#   define U_WORD_FMTD PRIu32
#   define U_LONG_FMTD PRIu64
#   define U_BYTE_FMTx PRIx8
#   define U_HALF_FMTx PRIx16
#   define U_WORD_FMTx PRIx32
#   define U_LONG_FMTx PRIx64
#   define U_BYTE_FMTX PRIX8
#   define U_HALF_FMTX PRIX16
#   define U_WORD_FMTX PRIX32
#   define U_LONG_FMTX PRIX64

typedef  int8_t  s_byte_t;
typedef  int16_t s_half_t;
typedef  int32_t s_word_t;
typedef  int64_t s_long_t;

#   define S_BYTE(n) INT8_C(n)
#   define S_HALF(n) INT16_C(n)
#   define S_WORD(n) INT32_C(n)
#   define S_LONG(n) INT64_C(n)

#   define S_BYTE_MIN INT8_MIN
#   define S_HALF_MIN INT16_MIN
#   define S_WORD_MIN INT32_MIN
#   define S_LONG_MIN INT64_MIN

#   define S_BYTE_MAX INT8_MAX
#   define S_HALF_MAX INT16_MAX
#   define S_WORD_MAX INT32_MAX
#   define S_LONG_MAX INT64_MAX

#   define S_BYTE_FMTD PRIi8
#   define S_HALF_FMTD PRIi16
#   define S_WORD_FMTD PRIi32
#   define S_LONG_FMTD PRIi64
#   define S_BYTE_FMTx PRIx8
#   define S_HALF_FMTx PRIx16
#   define S_WORD_FMTx PRIx32
#   define S_LONG_FMTx PRIx64
#   define S_BYTE_FMTX PRIX8
#   define S_HALF_FMTX PRIX16
#   define S_WORD_FMTX PRIX32
#   define S_LONG_FMTX PRIX64

struct cache_t;
struct cache_test_t;

#   define CACHE_FAILURE -1
#   define CACHE_SUCCESS  0
#   define CACHE_WAITING +1

#   define CACHE_TEST_FAILED  -1
#   define CACHE_TEST_PASSED   0
#   define CACHE_TEST_WAITING +1

struct cache_t {
  u_word_t   sr;
  u_word_t   tot_len;
  u_word_t   dat_len;
  u_byte_t * dat_buf;
  u_word_t   hdr_len;
  u_byte_t * hdr_buf;
  u_long_t   tagm;
  u_word_t   setm;
  u_word_t   datm;
  u_word_t   tags; /* in bits  */
  u_word_t   sets; /* in bits  */
  u_word_t   dats; /* in bits  */
  u_word_t   setc; /* in sets  */
  u_word_t   wayc; /* in ways  */
  u_word_t   datc; /* in bytes */
  u_word_t   tagz; /* in bits  */
  u_word_t   hdrc; /* in bytes */

  int ( * flush ) (
    _InOut struct cache_t * /* cache   */,
    _In    u_word_t         /* seti    */,
    _In    const u_byte_t * /* way_hdr */,
    _In    const u_byte_t * /* way_dat */
  );

  int ( * rp_reset ) (
    _InOut struct cache_t * /* cache   */,
    _In    const u_byte_t * /* set_hdr */,
    _In    const u_byte_t * /* set_dat */
  );

  int ( * rp_set ) (
    _InOut struct cache_t * /* cache   */,
    _In    const u_byte_t * /* set_hdr */,
    _In    const u_byte_t * /* set_dat */,
    _In    u_word_t         /* wayi    */
  );

  int ( * rp_get ) (
    _InOut struct cache_t * /* cache   */,
    _In    const u_byte_t * /* set_hdr */,
    _In    const u_byte_t * /* set_dat */,
    _Out   u_word_t *       /* wayi    */
  );
};

#   define cache_clr_ho(cache) (cache)->sr &= ~0x1
#   define cache_clr_hm(cache) (cache)->sr &= ~0x2
#   define cache_clr_sm(cache) (cache)->sr &= ~0x4
#   define cache_clr_wr(cache) (cache)->sr &= ~0x10
#   define cache_clr_wf(cache) (cache)->sr &= ~0x20

#   define cache_set_ho(cache) (cache)->sr |= 0x1
#   define cache_set_hm(cache) (cache)->sr |= 0x2
#   define cache_set_sm(cache) (cache)->sr |= 0x4
#   define cache_set_wr(cache) (cache)->sr |= 0x10
#   define cache_set_wf(cache) (cache)->sr |= 0x20

#   define cache_get_ho(cache) ((cache)->sr & 0x1)
#   define cache_get_hm(cache) ((cache)->sr & 0x2)
#   define cache_get_sm(cache) ((cache)->sr & 0x4)
#   define cache_get_wr(cache) ((cache)->sr & 0x10)
#   define cache_get_wf(cache) ((cache)->sr & 0x20)

struct cache_t * cache_ctor (
  _InOut struct cache_t * cache,
  _In    int              argc,
  _In    char **          argv
);

struct cache_t * cache_dtor (
  _InOut struct cache_t * cache
);

#   define cache_way_clr_valid(cache, way_hdr) (way_hdr)[0] &= ~0x1
#   define cache_way_clr_dirty(cache, way_hdr) (way_hdr)[0] &= ~0x2

#   define cache_way_set_valid(cache, way_hdr) (way_hdr)[0] |= 0x1
#   define cache_way_set_dirty(cache, way_hdr) (way_hdr)[0] |= 0x2

#   define cache_way_get_valid(cache, way_hdr) ((way_hdr)[0] & 0x1)
#   define cache_way_get_dirty(cache, way_hdr) ((way_hdr)[0] & 0x2)

void cache_way_set_tag (
  _InOut struct cache_t * cache,
  _InOut u_byte_t *       way_hdr,
  _In    u_long_t         tag
);

u_long_t cache_way_get_tag (
  _InOut struct cache_t * cache,
  _In    const u_byte_t * way_hdr
);

int cache_reset (
  _InOut struct cache_t * cache,
  _InOut u_word_t *       _seti
);

int cache_write (
  _InOut struct cache_t * cache,
  _In    u_long_t         adr,
  _In    u_word_t         len,
  _In    const u_byte_t * dat
);

int cache_read (
  _InOut struct cache_t * cache,
  _In    u_long_t         adr,
  _In    u_word_t         len,
  _Out   u_byte_t *       dat
);

int cache_flush (
  _InOut struct cache_t * cache,
  _InOut u_word_t *       _seti,
  _InOut u_word_t *       _wayi
);

struct cache_test_t {
  struct cache_t * cache;
  u_word_t         sr;
  u_word_t         setc;
  u_word_t *       setv;
};

#   define cache_test_clr_ho(test) (test)->sr &= ~0x1
#   define cache_test_clr_hm(test) (test)->sr &= ~0x2

#   define cache_test_set_ho(test) (test)->sr |= 0x1
#   define cache_test_set_hm(test) (test)->sr |= 0x2

#   define cache_test_get_ho(test) ((test)->sr & 0x1)
#   define cache_test_get_hm(test) ((test)->sr & 0x2)

struct cache_test_t * cache_test_ctor (
  _InOut struct cache_test_t * test,
  _InOut struct cache_t *      cache
);

struct cache_test_t * cache_test_dtor (
  _InOut struct cache_test_t * test
);

int cache_test_run (
  _InOut struct cache_test_t * test,
  _Out   FILE *                fp
);

# endif
