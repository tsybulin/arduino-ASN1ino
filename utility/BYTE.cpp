#include "tlv_tag.h"
#include "pt_type.h"
#include "pt_coder.h"

typedef byte BYTE_t ;

static const tlv_tag_t DEF_BYTE_tags[] = { (TAG_CLASS_APPLICATION | (1 << 2)) } ;

enc_rval_t BYTE_encode(type_descriptor_t *sd, void *ptr, char tag_mode, tlv_tag_t tag, app_consume_bytes_f *cb, void *app_key) {
    BYTE_t *st = (BYTE_t *)ptr ;
    enc_rval_t erval ;
    
    erval.encoded = write_tags(sd, 1, tag_mode, 0, tag, cb, app_key) ;
    if (erval.encoded == 0) {
        erval.failed_type = sd ;
        erval.structure_ptr = ptr ;
        return erval ;
    }

    if (cb) {
        if (cb(st, 1, app_key) == 0) {
            erval.encoded = -1 ;
            erval.failed_type = sd ;
            erval.structure_ptr = ptr;
            return erval ;
        }
    }

    erval.encoded += 1 ;
    ENCODED_OK(erval) ;
}

dec_rval_t BYTE_decode(codec_ctx_t *opt_codec_ctx, type_descriptor_t *td, void **nint_ptr, const void *buf_ptr, uint8_t size, char tag_mode) {
    BYTE_t *st = (BYTE_t *)*nint_ptr ;
    dec_rval_t rval ;
    char length ;

    if (st == NULL) {
        st = (BYTE_t *)(*nint_ptr = calloc(1, sizeof(*st))) ;
        if (st == NULL) {
            DECODE_FAILED ;
        }
    }

    rval = check_tags(opt_codec_ctx, td, 0, buf_ptr, size, tag_mode, 0, &length, 0) ;
    if (rval.code != RC_OK) {
        return rval ;
    }

    buf_ptr = ((const char *)buf_ptr) + rval.consumed ;
    size -= rval.consumed ;

    if (length > (tlv_len_t)size) {
        rval.code = RC_WMORE ;
        rval.consumed = 0 ;
        return rval ;
    }

    *st = ((const BYTE_t *)buf_ptr)[0] ;

    rval.code = RC_OK ;
    rval.consumed += length ;
    return rval ;
}

void BYTE_free(type_descriptor_t *td, void *sptr, uint8_t contents_only) {
    if (!td || !sptr) {
        return ;
    }

    if (!contents_only) {
        free(sptr) ;
    }
}

type_descriptor_t DEF_BYTE = {
    (anyfunc *) BYTE_free,
    (anyfunc *) BYTE_decode,
    (anyfunc *) BYTE_encode,
    DEF_BYTE_tags,
    1,
    DEF_BYTE_tags,
    1,
    0,
    0,
    0
} ;

