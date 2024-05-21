# include "cache.h"
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <time.h>

int my_flush (
  struct cache_t * cache,
  u_word_t         seti,
  const u_byte_t * way_hdr,
  const u_byte_t * way_buf
);

int my_rp_reset (
  struct cache_t * cache,
  u_byte_t *       set_hdr,
  u_byte_t *       set_buf
);

void my_rp_set (
  struct cache_t * cache,
  u_byte_t *       set_hdr,
  u_byte_t *       set_buf,
  u_word_t         wayi
);

int my_rp_get (
  struct cache_t * cache,
  const u_byte_t * set_hdr,
  const u_byte_t * set_buf,
  u_word_t *       wayi
);

int main (int argc, char ** argv)
{
  srand(time(NULL));

  struct cache_t cache;

  u_word_t hdrz = U_WORD(0);
  u_word_t adrz = U_WORD(48);
  u_word_t setz = U_WORD(2);
  u_word_t datz = U_WORD(1);

  cache.dats     = 0;
  cache.sets     = datz;
  cache.tags     = setz + datz;
  cache.setc     = U_WORD(1) << setz;
  cache.wayc     = 2;
  cache.datc     = U_WORD(1) << datz;
  cache.tagz     = adrz - cache.tags;
  hdrz += cache.tagz + U_WORD(8);
  cache.hdrc     = u_round_up(hdrz, U_WORD(8));
  printf("HEADER: %u BYTES, %u BITS\n", cache.hdrc, hdrz);
  cache.dat_buf  = NULL;
  cache.hdr_buf  = NULL;
  cache.flush    = (void *)my_flush;
  cache.rp_reset = (void *)my_rp_reset;
  cache.rp_set   = (void *)my_rp_set;
  cache.rp_get   = (void *)my_rp_get;

  if (cache_ctor(&cache, 0, NULL)) {
    cache_reset(&cache, NULL);
    cache_flush(&cache, NULL, NULL); /* print nothing */

    struct cache_test_t * test;
   
    if (test = cache_test_ctor(NULL, &cache)) {
      cache_test_run(test, stdout);
      cache_flush(&cache, NULL, NULL); /* print cache */
      cache_flush(&cache, NULL, NULL); /* print nothing */
      test = cache_test_dtor(test);
    }

    cache_dtor(&cache);
  }

  return 0;
}

int my_flush (
  struct cache_t * cache,
  u_word_t         seti,
  const u_byte_t * way_hdr,
  const u_byte_t * way_buf
)
{
  static u_word_t wayc = 0;

  if (!wayc) {
    fprintf(stdout, "\nSet %u\n", seti);
  }

  wayc = ++wayc & (cache->wayc - 1);

  u_word_t valid = cache_way_get_valid(cache, way_hdr);
  u_word_t dirty = cache_way_get_valid(cache, way_hdr);
  u_word_t ref   = (way_hdr[0] >> 2) & (cache->wayc - 1);
  u_long_t tag   = cache_way_get_tag(cache, way_hdr);

  fprintf(stdout, "%X [ %c %c %X 0x%016" U_LONG_FMTX " |",
    wayc,
    valid ? 'V' : 'v',
    dirty ? 'D' : 'd',
    ref,
    tag
  );

  u_word_t dati;

  for (dati = 0; dati < cache->datc; ++dati) {
    fprintf(stdout, " %02X", way_buf[dati]);
  }

  fprintf(stdout, " ]\n");

  return 0;
}

int my_rp_reset (
  struct cache_t * cache,
  u_byte_t *       set_hdr,
  u_byte_t *       set_buf
)
{
  u_word_t wayi;

  memset(set_hdr, 0, cache->hdr_len);
  memset(set_buf, 0, cache->dat_len);

  for (wayi = 0; wayi < cache->wayc; ++wayi) {
    u_byte_t * way_hdr = set_hdr + wayi * cache->hdrc;
    u_byte_t * way_buf = set_buf + wayi * cache->datc;
    
    way_hdr[0] = wayi << 2;
  }

  return 0;
}

void my_rp_set (
  struct cache_t * cache,
  u_byte_t *       set_hdr,
  u_byte_t *       set_buf,
  u_word_t         mru_wayi
)
{
  u_word_t refm = cache->wayc - 1;

  u_byte_t * mru_way_hdr = set_hdr + mru_wayi * cache->hdrc;
  u_word_t   mru_way_ref = (mru_way_hdr[0] >> 2) & refm;

  u_word_t wayi;

  for (wayi = 0; wayi < cache->wayc; ++wayi) {
    u_byte_t * way_hdr = set_hdr + wayi * cache->hdrc;
    u_word_t   way_ref = (way_hdr[0] >> 2) & refm;

    if (way_ref <= mru_way_ref)
      continue;

    --way_ref;

    way_hdr[0] &= ~(refm << 2);
    way_hdr[0] |= way_ref << 2;
  }

  mru_way_hdr[0] &= ~(refm << 2);
  mru_way_hdr[0] |= refm << 2;
}

int my_rp_get (
  struct cache_t * cache,
  const u_byte_t * set_hdr,
  const u_byte_t * set_buf,
  u_word_t *       _wayi
)
{
  u_word_t refm = cache->wayc - 1;
  u_word_t wayi;

  for (wayi = 0; wayi < cache->wayc; ++wayi) {
    u_byte_t * way_hdr = (u_byte_t *)set_hdr + wayi * cache->hdrc;
    u_word_t   way_ref = (way_hdr[0] >> 2) & refm;

    if (!way_ref) {
      *_wayi = wayi;
      return 0;
    }
  }

  return -1;
}
