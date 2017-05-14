#include "tlv_len.h"
#include "tlv_tag.h"

char tlv_len_serialize(tlv_len_t len, void *bufptr, uint8_t size) {
    uint8_t required_size ;
    uint8_t *buf = (uint8_t *)bufptr ;
    uint8_t *end;
    uint8_t i;
   
    if (len <= 127) {
        if (size) {
            *buf = (uint8_t)len ;
        }
        return 1 ;
    }

    for (required_size = 1, i = 8; i < 8 * sizeof(len); i += 8) {
        if (len >> i) {
            required_size++ ;
        } else {
            break ;
        }
    }

    if (size <= required_size) {
        return required_size + 1 ;
    }

    *buf++ = (uint8_t)(0x80 | required_size) ;
    
    end = buf + required_size ;
    for (i -= 8; buf < end; i -= 8, buf++) {
        *buf = (uint8_t)(len >> i) ;
    }

    return required_size + 1 ;
}

char tlv_len_fetch(boolean is_constructed, const void *bufptr, uint8_t size, tlv_len_t *len_r) {
    const uint8_t *buf = (const uint8_t *)bufptr ;
    unsigned oct ;
    
    if (size == 0) {
        return 0 ;
    }

    oct = *(const uint8_t *)buf;
    if ((oct & 0x80) == 0) {
        *len_r = oct ;
        return 1 ;
    } else {
        int len ;
        uint8_t skipped ;

        if (is_constructed && oct == 0x80) {
            *len_r = -1 ;
            return 1 ;
        }

        if (oct == 0xff) {
            return -1 ;
        }

        oct &= 0x7F ;
        for (len = 0, buf++, skipped = 1; oct && (++skipped <= size); buf++, oct--) {
            len = (len << 8) | *buf ;
            if (len < 0 || (len >> ((8 * sizeof(len)) - 8) && oct > 1)) {
                return -1 ;
            }
        }

        if (oct == 0) {
            int lenplusepsilon = (uint8_t)len + 1024 ;
            if (lenplusepsilon < 0) {
                return -1 ;
            }

            *len_r = len ;
            return skipped ;
        }

        return 0 ;
    }
}

char tlv_len_skip(struct codec_ctx_s *opt_codec_ctx, boolean is_constructed, const void *bufptr, uint8_t size) {
    tlv_len_t vlen ;
    char tl ;
    char ll ;
    uint8_t skip ;

    ll = tlv_len_fetch(is_constructed, bufptr, size, &vlen) ;
    if (ll <= 0) {
        return ll ;
    }

    if (vlen < 0xff) {
        skip = ll + vlen ;
        if (skip > size) {
            return 0 ;
        }
        return skip ;
    }
    
    for(skip = ll, bufptr = ((const char *)bufptr) + ll, size -= ll;;) {
        tlv_tag_t tag ;
        tl = tlv_tag_fetch(bufptr, size, &tag) ;
        if (tl <= 0) {
            return tl ;
        }

        ll = tlv_len_skip(opt_codec_ctx, TLV_CONSTRUCTED(bufptr), ((const char *)bufptr) + tl, size - tl) ;
        if (ll <= 0) {
            return ll ;
        }

        skip += tl + ll ;

        if (((const uint8_t *)bufptr)[0] == 0 && ((const uint8_t *)bufptr)[1] == 0) {
            return skip ;
        }

        bufptr = ((const char *)bufptr) + tl + ll ;
        size -= tl + ll ;
    }
}

