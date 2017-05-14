#include "pt_coder.h"

#include <alloca.h>
#include "tlv_len.h"
#include "pt_type.h"

#undef  ADVANCE
#define ADVANCE(num_bytes) do { uint8_t num = num_bytes; ptr = ((const char *)ptr) + num; size -= num; consumed_myself += num; } while(0)
#undef  RETURN
#define RETURN(_code) do { dec_rval_t rval; rval.code = _code; if (opt_ctx) opt_ctx->step = step; if (_code == RC_OK || opt_ctx) rval.consumed = consumed_myself; else rval.consumed = 0; return rval; } while(0)

char write_TL(tlv_tag_t tag, tlv_len_t len, app_consume_bytes_f *cb, void *app_key, uint8_t constructed) {
    uint8_t buf[32] ;
    uint8_t size = 0 ;
    uint8_t buf_size = cb ? sizeof(buf) : 0 ;
    char tmp ;

    tmp = tlv_tag_serialize(tag, buf, buf_size) ;
    if (tmp == -1 || tmp > (uint8_t)sizeof(buf)) {
        return -1 ;
    }
    size += tmp ;

    tmp = tlv_len_serialize(len, buf+size, buf_size ? buf_size-size : 0) ;
    if (tmp == -1) {
        return -1 ;
    }
    size += tmp ;

    if (size > sizeof(buf)) {
        return -1 ;
    }

    if (cb) {
        if (constructed) {
            *buf |= 0x20 ;
        }
        if (cb(buf, size, app_key) < 0) {
            return -1 ;
        }
    }

    return size ;
}

char write_tags(
    struct type_descriptor_s *type_descriptor,
    uint8_t struct_length,
    char tag_mode,
    uint8_t last_tag_form,
    tlv_tag_t tag,
    app_consume_bytes_f *consume_bytes_cb,
    void *app_key
) {
    const tlv_tag_t *tags ;
    uint8_t tags_count ;
    uint8_t overall_length ;
    char *lens ;
    int i ;

    if (tag_mode) {
        char stag_offset ;
        tlv_tag_t *tags_buf ;
        tags_buf = (tlv_tag_t *)alloca((type_descriptor->tags_count + 1) * sizeof(tlv_tag_t)) ;
        if (!tags_buf) {
            return -1 ;
        }
        tags_count = type_descriptor->tags_count + 1 - ((tag_mode == -1) && type_descriptor->tags_count) ;
        tags_buf[0] = tag ;
        stag_offset = -1 + ((tag_mode == -1) && type_descriptor->tags_count) ;
        for (i = 1; i < tags_count; i++) {
            tags_buf[i] = type_descriptor->tags[i + stag_offset] ;
        }
        tags = tags_buf ;
    } else {
        tags = type_descriptor->tags ;
        tags_count = type_descriptor->tags_count ;
    }

    if (tags_count == 0) {
        return 0 ;
    }

    lens = (char *)alloca(tags_count * sizeof(lens[0])) ;
    if (!lens) {
        return -1 ;
    }

    overall_length = struct_length ;
    for (i = tags_count - 1; i >= 0; --i) {
        lens[i] = write_TL(tags[i], overall_length, 0, 0, 0) ;
        if (lens[i] == -1) {
            return -1 ;
        }
        overall_length += lens[i] ;
        lens[i] = overall_length - lens[i] ;
    }

    if (!consume_bytes_cb) {
        return overall_length - struct_length ;
    }

    for(i = 0; i < tags_count; i++) {
            char len ;
            uint8_t constr ;
            constr = (last_tag_form || i < (tags_count - 1)) ;
            len = write_TL(tags[i], lens[i], consume_bytes_cb, app_key, constr) ;
            if (len == -1) {
                return -1 ;
            }
    }

    return overall_length - struct_length ;
}

enc_rval_t encode_primitive(type_descriptor_t *td, void *sptr, char tag_mode, tlv_tag_t tag, app_consume_bytes_f *cb, void *app_key) {
    enc_rval_t erval ;
    PRIMITIVE_TYPE_t *st = (PRIMITIVE_TYPE_t *)sptr ;
    erval.encoded = write_tags(td, st->size, tag_mode, 0, tag, cb, app_key) ;
    if (erval.encoded == -1) {
        erval.failed_type = td ;
        erval.structure_ptr = sptr ;
        return erval ;
    }

    if (cb && st->buf) {
        if(cb(st->buf, st->size, app_key) < 0) {
                erval.encoded = -1;
                erval.failed_type = td;
                erval.structure_ptr = sptr;
                return erval;
        }
    } else {
        //assert(st->buf || st->size == 0) ;
    }

    erval.encoded += st->size ;
    ENCODED_OK(erval) ;
}

void primitive_free(type_descriptor_t *td, void *sptr, uint8_t contents_only) {
    PRIMITIVE_TYPE_t *st = (PRIMITIVE_TYPE_t *)sptr ;
    
    if (!td || !sptr) {
        return ;
    }
    
    if (st->buf) {
        free(st->buf) ;
    }
    
    if(!contents_only) {
        free(st) ;
    }
}

enc_rval_t encode(type_descriptor_t *type_descriptor, void *struct_ptr, app_consume_bytes_f *consume_bytes, void *app_key) {
    return ((type_encoder_f*)type_descriptor->encoder)(type_descriptor, struct_ptr, 0, 0, consume_bytes, app_key) ;
}

dec_rval_t check_tags(
    codec_ctx_t *opt_codec_ctx, type_descriptor_t *td, struct_ctx_t *opt_ctx,
    const void *ptr, uint8_t size, char tag_mode, char last_tag_form,
    char *last_length, uint8_t *opt_tlv_form
) {
    uint8_t consumed_myself = 0 ;
    char tag_len;
    char len_len;
    tlv_tag_t tlv_tag ;
    tlv_len_t tlv_len ;
    tlv_len_t limit_len = -1 ;
    uint8_t expect_00_terminators = 0 ;
    int tlv_constr = -1 ;
    uint8_t step = opt_ctx ? opt_ctx->step : 0 ;
    int tagno ;

    if (opt_codec_ctx->max_stack_size < 1) {
        RETURN(RC_FAIL) ;
    }

    tagno = step + (tag_mode == 1 ? -1 : 0) ;
    if (tag_mode == 0 && tagno == td->tags_count) {
        tag_len = tlv_tag_fetch(ptr, size, &tlv_tag) ;
        if (tag_len == 0) {
            RETURN(RC_WMORE) ;
        }
        
        tlv_constr = TLV_CONSTRUCTED(ptr) ;
        len_len = tlv_len_fetch(tlv_constr, (const char *)ptr + tag_len, size - tag_len, &tlv_len) ;
        if (len_len == 0) {
            RETURN(RC_WMORE) ;
        }

        ADVANCE(tag_len + len_len) ;
    } else {
        // assert(tagno < td->tags_count) ;
    }

    for ((void)tagno; tagno < td->tags_count; tagno++, step++) {
        tag_len = tlv_tag_fetch(ptr, size, &tlv_tag) ;
        if (tag_len == 0) {
            RETURN(RC_WMORE) ;
        }

        tlv_constr = TLV_CONSTRUCTED(ptr) ;

        if (tag_mode != 0 && step == 0) {
           // not expected here 
        } else {
            if (tlv_tag != td->tags[tagno]) {
                RETURN(RC_FAIL) ;
            }
        }

        if (tagno < (td->tags_count - 1)) {
            if (tlv_constr == 0) {
                RETURN(RC_FAIL) ;
            }
        } else {
            if (last_tag_form != tlv_constr && last_tag_form != -1) {
                RETURN(RC_FAIL) ;
            }
        }

        len_len = tlv_len_fetch(tlv_constr, (const char *)ptr + tag_len, size - tag_len, &tlv_len) ;
        if (len_len == 0) {
            RETURN(RC_WMORE) ;
        }

        if (tlv_len == 0xff) {
            if (limit_len == 0xff) {
                expect_00_terminators++ ;
            } else {
                RETURN(RC_FAIL) ;
            }

            ADVANCE(tag_len + len_len) ;
            continue ;
        } else {
            if (expect_00_terminators) {
                RETURN(RC_FAIL) ;
            }
        }

        if (limit_len == 0xff) {
            limit_len = tlv_len + tag_len + len_len ;
            if (limit_len == 0xff) {
                RETURN(RC_FAIL) ;
            }
        } else if (limit_len != tlv_len + tag_len + len_len) {
                RETURN(RC_FAIL) ;
        }

        ADVANCE(tag_len + len_len) ;
        limit_len -= (tag_len + len_len) ;
        if (size > limit_len) {
            size = limit_len ;
        }
    }

    if (opt_tlv_form) {
        *opt_tlv_form = tlv_constr ;
    }
    
    if (expect_00_terminators) {
        *last_length = -expect_00_terminators ;
    } else {
        *last_length = tlv_len ;
    }

    RETURN(RC_OK) ;
}

dec_rval_t decode_primitive(codec_ctx_t *opt_codec_ctx, type_descriptor_t *td, void **sptr, const void *buf_ptr, uint8_t size, char tag_mode) {
    PRIMITIVE_TYPE_t *st = (PRIMITIVE_TYPE_t *)*sptr ;
    dec_rval_t rval ;
    char length = 0 ;
    
    if (st == NULL) {
        st = (PRIMITIVE_TYPE_t *)calloc(1, sizeof(*st)) ;
        if (st == NULL) {
            DECODE_FAILED ;
        }
        *sptr = (void *)st ;
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

    st->size = (int)length ;
    if (sizeof(st->size) != sizeof(length) && (tlv_len_t)st->size != length) {
        st->size = 0 ;
        DECODE_FAILED ;
    }

    st->buf = (uint8_t *)malloc(length + 1) ;
    if (!st->buf) {
        st->size = 0 ;
        DECODE_FAILED ;
    }

    memcpy(st->buf, buf_ptr, length) ;
    st->buf[(byte)length] = '\0' ;
    rval.code = RC_OK ;
    rval.consumed += length ;
    
    return rval ;
}

dec_rval_t decode(codec_ctx_t *opt_codec_ctx, type_descriptor_t *type_descriptor, void **struct_ptr, const void *ptr, uint8_t size) {
    codec_ctx_t s_codec_ctx ;

    if (opt_codec_ctx) {
        if (opt_codec_ctx->max_stack_size) {
            s_codec_ctx = *opt_codec_ctx ;
            opt_codec_ctx = &s_codec_ctx ;
        }
    } else {
        memset(&s_codec_ctx, 0, sizeof(s_codec_ctx)) ;
        s_codec_ctx.max_stack_size = DEFAULT_STACK_MAX ;
        opt_codec_ctx = &s_codec_ctx ;
    }

    return ((type_decoder_f*)type_descriptor->decoder)(opt_codec_ctx, type_descriptor, struct_ptr, ptr, size, 0) ;
}

