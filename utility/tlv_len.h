#ifndef TLV_LEN_H
#define TLV_LEN_H

#include <Arduino.h>
#include <stdint.h>

typedef uint8_t tlv_len_t ;

char tlv_len_serialize(tlv_len_t len, void *bufptr, uint8_t size) ;
char tlv_len_fetch(boolean is_constructed, const void *bufptr, uint8_t size, tlv_len_t *len_r) ;
char tlv_len_skip(struct codec_ctx_s *opt_codec_ctx, boolean is_constructed, const void *bufptr, uint8_t size) ;

#endif

