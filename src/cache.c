# include "cache.h"
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>

struct cache_t * cache_ctor (
  _InOut struct cache_t * cache,
  _In    int              argc,
  _In    char **          argv
)
{
  if (!cache) {
    cache = (struct cache_t *)malloc(
      sizeof(struct cache_t)
    );

    if (!cache)
      return cache;

    cache->sr      = 0;
    cache->dat_buf = NULL;
    cache->hdr_buf = NULL;
    cache_set_ho(cache);
  } else {
    cache->sr = 0;
  }

  for (int argi = 0; argi < argc; ++argi) {
    char * args = argv[argi];

    if (
      0 == strcmp(args, "-h")     ||
      0 == strcmp(args, "--help")
    ) {
      fprintf(
        stdout,
        "usage: cache [options]\n"
        "usage: cache -h\n"
        "\n"
        "options:\n"
        "  -h, --help            --- Print the help page.\n"
        "  -v, --version         --- Print the version.\n"
        "  -g, --geom   GEOMETRY --- Set the cache geometry.\n"
        "  -f, --flush  METHOD   --- Set the cache flush method.\n"
        "  -p, --policy METHODS  --- Set the cache policy methods.\n"
        "\n"
      );

      cache = cache_dtor(cache);
      return NULL;
    }

    if (
      0 == strcmp(args, "-v")        ||
      0 == strcmp(args, "--version")
    ) {
      fprintf(
        stdout,
        "Cache v%u.%u.%u\n"
        "Built on %s at %s.\n"
        "\n",
        __CACHE_VERSION_MAJOR,
        __CACHE_VERSION_MINOR,
        __CACHE_VERSION_PATCH,
        __DATE__,
        __TIME__
      );

      cache = cache_dtor(cache);
      return NULL;
    }

    if (
      0 == strcmp(args, "-g")     ||
      0 == strcmp(args, "--geom")
    ) {
      u_word_t hdrz, adrz, setz, datz;

      /* TODO: extract information */

      cache->dats = 0;
      cache->sets = datz;
      cache->tags = setz + datz;
      cache->setc = U_WORD(1) << setz;
      cache->datc = U_WORD(1) << datz;
      cache->tagz = adrz - cache->tags;
      hdrz += cache->tagz + U_WORD(8);
      cache->hdrc = u_round_up(hdrz, U_WORD(8));
    } else if (
      0 == strcmp(args, "-f")      ||
      0 == strcmp(args, "--flush")
    ) {
      /* TODO: extract information */
    } else if (
      0 == strcmp(args, "-p")      ||
      0 == strcmp(args, "--policy")
    ) {
      /* TODO: extract information */
    }
  }

  cache->tagm    = (U_LONG(1) << cache->tagz) - U_LONG(1);
  cache->setm    = cache->setc - U_WORD(1);
  cache->datm    = cache->datc - U_WORD(1);
  cache->dat_len = cache->wayc * cache->datc;
  cache->hdr_len = cache->wayc * cache->hdrc;
  cache->tot_len = cache->setc * (
    cache->dat_len + cache->hdr_len
  );

  if (!cache->dat_buf) {
    cache->dat_buf = (u_byte_t *)malloc(
      cache->hdr_buf ? cache->dat_len : cache->tot_len
    );

    if (!cache->dat_buf) {
      cache = cache_dtor(cache);
      return NULL;
    }

    cache_set_hm(cache);
  }

  if (!cache->hdr_buf) {
    cache->hdr_buf = cache->dat_buf + (
      cache->setc * cache->dat_len
    );

    cache_set_sm(cache);
  }

  return cache;
}

struct cache_t * cache_dtor (
  _InOut struct cache_t * cache
)
{
  if (!cache)
    return cache;

  if (cache_get_hm(cache)) {
    if (!cache->dat_buf) {
      free(cache->dat_buf);
      cache->dat_buf = NULL;
    }

    if (!cache_get_sm(cache)) {
      cache->hdr_buf = NULL;
    }
  }

  if (cache_get_ho(cache)) {
    free(cache);
    cache = NULL;
  }

  return cache;
}

void cache_way_set_tag (
  _InOut struct cache_t * cache,
  _InOut u_byte_t *       way_hdr,
  _In    u_long_t         tag
)
{
  u_byte_t * hdr = (u_byte_t *)way_hdr + 1;

  u_word_t tags = cache->tagz / U_WORD(8);
  u_word_t tagr = cache->tagz % U_WORD(8);
  u_word_t tagi;

  if (tagr) {
    u_byte_t tagm = (U_BYTE(1) << tagr) - U_BYTE(1);

    hdr[tags] = (u_byte_t)(
      tag >> (cache->tagz - tagr)
    ) & tagm;
  }

  for (tagi = U_WORD(0); tagi < tags; ++tagi) {
    hdr[tagi] = (u_byte_t)tag;
    tag >>= 8;
  }
}

u_long_t cache_way_get_tag (
  _InOut struct cache_t * cache,
  _In    const u_byte_t * way_hdr
)
{
  u_byte_t * hdr = (u_byte_t *)way_hdr + 1;

  u_long_t tag  = U_LONG(0);
  u_word_t tags = cache->tagz / U_WORD(8);
  u_word_t tagr = cache->tagz % U_WORD(8);
  u_word_t tagi;

  for (tagi = tags; tagi > 0; tagi) {
    tag <<= 8;
    tag  |= hdr[--tagi];
  }

  if (!tagr)
    return tag;

  u_byte_t tagm = (U_BYTE(1) << tagr) - U_BYTE(1);

  tag |= (
    (u_long_t)(hdr[tags] & tagm) << (cache->tagz - tagr)
  );

  return tag;
}

int cache_reset (
  _InOut struct cache_t * cache,
  _InOut u_word_t *       _seti
)
{
  u_word_t seti = _seti ? *_seti : U_WORD(0);

  for (seti; seti < cache->setc; ++seti) {
    u_byte_t * set_hdr = cache->hdr_buf + seti * cache->hdr_len;
    u_byte_t * set_dat = cache->dat_buf + seti * cache->dat_len;

    int res = cache->rp_reset(cache, set_hdr, set_dat);

    if (res < 0)
      return CACHE_FAILURE;

    if (!res)
      continue;

    if (_seti) {
      *_seti = seti;
    }

    cache_set_wr(cache);
    return CACHE_WAITING;
  }

  return CACHE_SUCCESS;
}

int cache_write (
  _InOut struct cache_t * cache,
  _In    u_long_t         adr,
  _In    u_word_t         len,
  _In    const u_byte_t * dat
)
{
  if (cache_get_wr(cache) || cache_get_wf(cache))
    return CACHE_WAITING;

  u_long_t tag  = (u_long_t)(adr >> cache->tags) & cache->tagm;
  u_word_t seti = (u_word_t)(adr >> cache->sets) & cache->setm;
  u_word_t wayi;
  u_word_t dati = (u_word_t)(adr >> cache->dats) & cache->datm;

  if (!len || cache->datc < dati + len) {
    len = cache->datc - dati;
  }

  u_byte_t * set_hdr = cache->hdr_buf + seti * cache->hdr_len;
  u_byte_t * set_dat = cache->dat_buf + seti * cache->dat_len;

  for (wayi = U_WORD(0); wayi < cache->wayc; ++wayi) {
    u_byte_t * way_hdr = set_hdr + wayi * cache->hdrc;
    u_byte_t * way_dat = set_dat + wayi * cache->datc;

    if (!cache_way_get_valid(cache, way_hdr))
      continue;

    u_long_t way_tag = cache_way_get_tag(cache, way_hdr);

    if (way_tag != tag)
      continue;

    memcpy(way_dat + dati, dat, len);
    cache_way_set_dirty(cache, way_hdr);
    cache->rp_set(cache, set_hdr, set_dat, wayi);

    return CACHE_SUCCESS;
  }

  for (wayi = U_WORD(0); wayi < cache->wayc; ++wayi) {
    u_byte_t * way_hdr = set_hdr + wayi * cache->hdrc;
    u_byte_t * way_dat = set_dat + wayi * cache->datc;

    if (cache_way_get_valid(cache, way_hdr))
      continue;

    memcpy(way_dat + dati, dat, len);
    cache_way_set_valid(cache, way_hdr);
    cache_way_set_dirty(cache, way_hdr);
    cache_way_set_tag(cache, way_hdr, tag);
    cache->rp_set(cache, set_hdr, set_dat, wayi);

    return CACHE_SUCCESS;
  }

  int res = cache->rp_get(cache, set_hdr, set_dat, &wayi);

  if (res)
    return CACHE_FAILURE;

  u_byte_t * way_hdr = set_hdr + wayi * cache->hdrc;
  u_byte_t * way_dat = set_dat + wayi * cache->datc;

  memcpy(way_dat + dati, dat, len);
  cache_way_set_valid(cache, way_hdr);
  cache_way_set_dirty(cache, way_hdr);
  cache_way_set_tag(cache, way_hdr, tag);
  cache->rp_set(cache, set_hdr, set_dat, wayi);

  return CACHE_SUCCESS;
}

int cache_read (
  _InOut struct cache_t * cache,
  _In    u_long_t         adr,
  _In    u_word_t         len,
  _Out   u_byte_t *       dat
)
{
  if (cache_get_wr(cache) || cache_get_wf(cache))
    return CACHE_WAITING;

  u_long_t tag  = (u_long_t)(adr >> cache->tags) & cache->tagm;
  u_word_t seti = (u_word_t)(adr >> cache->sets) & cache->setm;
  u_word_t wayi;
  u_word_t dati = (u_word_t)(adr >> cache->dats) & cache->datm;

  if (!len || cache->datc < dati + len) {
    len = cache->datc - dati;
  }

  u_byte_t * set_hdr = cache->hdr_buf + seti * cache->hdr_len;
  u_byte_t * set_dat = cache->dat_buf + seti * cache->dat_len;

  for (wayi = U_WORD(0); wayi < cache->wayc; ++wayi) {
    u_byte_t * way_hdr = set_hdr + wayi * cache->hdrc;
    u_byte_t * way_dat = set_dat + wayi * cache->datc;

    if (!cache_way_get_valid(cache, way_hdr))
      continue;

    u_long_t way_tag = cache_way_get_tag(cache, way_hdr);

    if (way_tag != tag)
      continue;

    memcpy(dat, way_dat + dati, len);
    cache->rp_set(cache, set_hdr, set_dat, wayi);

    return CACHE_SUCCESS;
  }

  return CACHE_FAILURE;
}

int cache_flush (
  _InOut struct cache_t * cache,
  _InOut u_word_t *       _seti,
  _InOut u_word_t *       _wayi
)
{
  u_word_t seti = _seti ? *_seti : U_WORD(0);
  u_word_t wayi = _wayi ? *_wayi : U_WORD(0);

  for (seti; seti < cache->setc; ++seti) {
    u_byte_t * set_hdr = cache->hdr_buf + seti * cache->hdr_len;
    u_byte_t * set_dat = cache->dat_buf + seti * cache->dat_len;

    for (wayi; wayi < cache->wayc; ++wayi) {
      u_byte_t * way_hdr = set_hdr + wayi * cache->hdrc;
      u_byte_t * way_dat = set_dat + wayi * cache->datc;

      if (!cache_way_get_dirty(cache, way_hdr))
        continue;

      int res = cache->flush(cache, seti, way_hdr, way_dat);

      if (res < 0)
        return CACHE_FAILURE;

      if (!res) {
        cache_way_clr_dirty(cache, way_hdr);
        continue;
      }

      if (_seti) {
        *_seti = seti;
      }

      if (_wayi) {
        *_wayi = wayi;
      }

      cache_set_wf(cache);
      return CACHE_WAITING;
    }

    wayi = U_WORD(0);
  }

  return CACHE_SUCCESS;
}

struct cache_test_t * cache_test_ctor (
  _InOut struct cache_test_t * test,
  _InOut struct cache_t *      cache
)
{
  if (!cache)
    return NULL;

  if (!test) {
    test = (struct cache_test_t *)malloc(
      sizeof(struct cache_test_t)
    );

    if (!test)
      return test;

    test->sr   = 0;
    test->setv = NULL;
    cache_test_set_ho(test);
  } else {
    if (cache->setc != test->setc)
      return NULL;

    test->sr = 0;
  }

  if (!test->setv) {
    test->setv = (u_word_t *)calloc(
      sizeof(u_word_t), cache->setc
    );

    if (!test->setv) {
      test = cache_test_dtor(test);
      return NULL;
    }

    cache_test_set_hm(test);
  }

  test->setc  = cache->setc;
  test->cache = cache;

  return test;
}

struct cache_test_t * cache_test_dtor (
  _InOut struct cache_test_t * test
)
{
  if (!test)
    return test;

  if (cache_test_get_hm(test)) {
    if (test->setv) {
      free(test->setv);
      test->setv = NULL;
    }
  }

  if (cache_test_get_ho(test)) {
    free(test);
    test = NULL;
  }

  return test;
}

static u_long_t _cache_test_rand_adr (
  _InOut struct cache_test_t * test,
  _In    const u_word_t *      _seti,
  _In    const u_word_t *      _dati
);

static void _cache_test_rand_dat (
  _InOut struct cache_test_t * test,
  _In    u_long_t              adr,
  _InOut u_word_t *            len,
  _Out   u_byte_t *            buf
);

static void _cache_test_print (
  _Out FILE *       fp,
  _In  const char * fmt,
  _In               ...
);

int cache_test_run (
  _InOut struct cache_test_t * test,
  _Out   FILE *                fp
)
{
  if (cache_get_wr(test->cache) || cache_get_wf(test->cache))
    return CACHE_TEST_WAITING;

  _cache_test_print(fp, "TEST CACHE\n");
  _cache_test_print(fp, ".---------\n");

  u_word_t seti, wayi;

  const u_word_t N = test->cache->datc;
  u_byte_t wr_buf [N];
  u_byte_t rd_buf [N];

  const u_word_t M = test->cache->wayc;
  u_word_t cc_res [M];

  for (seti = U_WORD(0); seti < test->setc; ++seti) {
    _cache_test_print(
      fp,
      "| SET %" U_WORD_FMTD "\n",
      seti
    );

    memset(cc_res, 0, M * sizeof(u_word_t));

    /* test: write/read */

    for (wayi = U_WORD(0); wayi < M; ++wayi) {
      _cache_test_print(
        fp,
        "| | WAY %" U_WORD_FMTD "\n",
        wayi
      );

      u_long_t adr = _cache_test_rand_adr(test, &seti, NULL);

      memset(wr_buf, 0, N);
      memset(rd_buf, 0, N);

      u_word_t len = U_WORD(0);
      _cache_test_rand_dat(test, adr, &len, wr_buf);

      int wr_res = cache_write(test->cache, adr, len, wr_buf);

      _cache_test_print(
        fp,
        "| | | WRITE %" U_WORD_FMTD " BYTES AT 0x%" U_LONG_FMTX ": %d\n",
        len, adr, wr_res
      );

      int rd_res = cache_read(test->cache, adr, len, rd_buf);

      _cache_test_print(
        fp,
        "| | | READ  %" U_WORD_FMTD " BYTES AT 0x%" U_LONG_FMTX ": %d\n",
        len, adr, rd_res
      );

      int cc = memcmp(wr_buf, rd_buf, len);

      _cache_test_print(
        fp,
        "| | | COMP  %" U_WORD_FMTD " BYTES: %d\n",
        len, cc
      );

      cc_res[wayi] |= (u_word_t)(wr_res & 0xFF) <<  0;
      cc_res[wayi] |= (u_word_t)(rd_res & 0xFF) <<  8;
      cc_res[wayi] |= (u_word_t)(cc     & 0xFF) << 16;
    }

    /* test: replace policy */

    for (wayi = U_WORD(0); wayi < M; ++wayi) {
      _cache_test_print(
        fp,
        "| | WAY %" U_WORD_FMTD "\n",
        wayi
      );

      u_long_t adr = _cache_test_rand_adr(test, &seti, NULL);

      memset(wr_buf, 0, N);
      memset(rd_buf, 0, N);

      u_word_t len = U_WORD(0);
      _cache_test_rand_dat(test, adr, &len, wr_buf);

      int wr_res = cache_write(test->cache, adr, len, wr_buf);

      _cache_test_print(
        fp,
        "| | | REPL  %" U_WORD_FMTD " BYTES AT 0x%" U_LONG_FMTX ": %d\n",
        len, adr, wr_res
      );

      cc_res[wayi] |= (u_word_t)(wr_res & 0xFF) << 24;
    }

    /* verify test */

    for (wayi = U_WORD(0); wayi < M; ++wayi) {
      test->setv[seti] += cc_res[wayi];
      _cache_test_print(
        fp,
        "| | CHECK WAY %" U_WORD_FMTD ": 0x%06X\n",
        wayi, cc_res[wayi]
      );
    }
  }

  for (seti = U_WORD(0); seti < test->setc; ++seti) {
    if (test->setv[seti]) {
      _cache_test_print(fp, "| TEST FAILED\n");
      return CACHE_TEST_FAILED;
    }
  }

  _cache_test_print(fp, "| TEST PASSED\n");
  return CACHE_TEST_PASSED;
}

static u_long_t _cache_test_rand_adr (
  _InOut struct cache_test_t * test,
  _In    const u_word_t *      _seti,
  _In    const u_word_t *      _dati
)
{
  u_word_t seti = _seti ? *_seti : ((u_word_t)rand() & test->cache->setm);
  u_word_t dati = _dati ? *_dati : ((u_word_t)rand() & test->cache->datm);
  u_word_t tagl = (u_word_t)rand();
  u_word_t tagh = (u_word_t)rand();
  u_long_t tag  = (
    ((u_long_t)tagh << 32) | tagl
  ) & test->cache->tagm;

  u_long_t adr  = (
    (tag  << test->cache->tags) |
    (seti << test->cache->sets) |
    (dati << test->cache->dats)
  );

  return adr;
}

static void _cache_test_rand_dat (
  _InOut struct cache_test_t * test,
  _In    u_long_t              adr,
  _InOut u_word_t *            len,
  _Out   u_byte_t *            buf
)
{
  u_word_t dati = (u_word_t)(adr >> test->cache->dats) & test->cache->datm;

  if (!*len || test->cache->datc < dati + *len) {
    *len = test->cache->datc - dati;
  }

  u_word_t idx;

  for (idx = U_WORD(0); idx < *len; ++idx) {
    buf[idx] = (u_byte_t)rand();
  }
}

static void _cache_test_print (
  _Out FILE *       fp,
  _In  const char * fmt,
  _In               ...
)
{
  if (!fp)
    return;

  va_list ap;
  va_start(ap, fmt);
  vfprintf(fp, fmt, ap);
  va_end(ap);
}
