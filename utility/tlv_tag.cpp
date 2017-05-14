#include "tlv_tag.h"

char tlv_tag_serialize(tlv_tag_t tag, void *bufptr, uint8_t size) {
    byte tclass = TAG_CLASS(tag) ;
    tlv_tag_t tval = TAG_VALUE(tag) ;
    uint8_t *buf = (uint8_t *)bufptr ;
    uint8_t *end ;
    uint8_t required_size ;
    uint8_t i ;

    if (tval <= 30) {
        if (size) {
            buf[0] = (tclass << 6) | tval ;
        }
        return 1 ;
    } else if (size) {
        *buf++ = (tclass << 6) | 0x1F ;
        size-- ;
    }

    for (required_size = 1, i = 7; i < 8 * sizeof(tval); i += 7) {
        if (tval >> i) {
            required_size++ ;
        } else {
            break ;
        }
    }

    if (size < required_size) {
        return required_size + 1 ;
    }

    end = buf + required_size - 1 ;

    for (i -= 7; buf < end; i -= 7, buf++) {
        *buf = 0x80 | ((tval >> i) & 0x7F) ;
    }
    
    *buf = (tval & 0x7F) ;
    
    return required_size + 1 ;    

}

char tlv_tag_fetch(const void *bufptr, uint8_t size, tlv_tag_t *tag_r) {
    unsigned val ;
    tlv_tag_t tclass ;
    uint8_t skipped ;

    if (size == 0) {
        return 0 ;
    }

    val = *(const uint8_t *)bufptr ;
    tclass = (val >> 6) ;
    if ((val &= 0x1F) != 0x1F) {
        *tag_r = (val << 2) | tclass ;
        return 1 ;
    }

    for (val = 0, bufptr = ((const char *)bufptr) + 1, skipped = 2; skipped <= size; bufptr = ((const char *)bufptr) + 1, skipped++) {
        unsigned int oct = *(const uint8_t *)bufptr ;
        if (oct & 0x80) {
            val = (val << 7) | (oct & 0x7F) ;
            if (val >> ((8 * sizeof(val)) - 9)) {
                return -1 ;
            }
        } else {
            val = (val << 7) | oct ;
            *tag_r = (val << 2) | tclass ;
            return skipped ;
        }
    }
    
    return 0 ;
}

