#include "SEQUENCE.h"

#include "tlv_tag.h"
#include "tlv_len.h"
#include "pt_type.h"
#include "pt_coder.h"

#undef  RETURN
#define RETURN(_code) do { rval.code = _code; rval.consumed = consumed_myself; return rval; } while(0)
#undef  ADVANCE
#define ADVANCE(num_bytes) do { uint8_t num = num_bytes; ptr = ((const char *)ptr) + num; size -= num; if (ctx->left >= 0) ctx->left -= num; consumed_myself += num; } while(0)
#undef  NEXT_PHASE
#undef  PHASE_OUT
#define NEXT_PHASE(ctx) do { ctx->phase++; ctx->step = 0; } while(0)
#define PHASE_OUT(ctx)  do { ctx->phase = 10; } while(0)
#define IN_EXTENSION_GROUP(specs, memb_idx) (((memb_idx) > (specs)->ext_after) &&((memb_idx) < (specs)->ext_before))
#define SIZE_VIOLATION (ctx->left >= 0 && (uint8_t)ctx->left <= size)
#define LEFT ((size<(uint8_t)ctx->left) ? size : (uint8_t)ctx->left)
#define STRUCT_FREE(type_DEF, ptr) ((struct_free_f*)(type_DEF).free_struct)(&(type_DEF),ptr,0)
#define STRUCT_FREE_CONTENTS_ONLY(type_DEF, ptr) ((struct_free_f*)(type_DEF).free_struct)(&(type_DEF),ptr,1)

static int t2e_cmp(const void *ap, const void *bp) {
    const type_tag2member_t *a = (const type_tag2member_t *)ap ;
    const type_tag2member_t *b = (const type_tag2member_t *)bp ;

    uint8_t a_class = TAG_CLASS(a->el_tag) ;
    uint8_t b_class = TAG_CLASS(b->el_tag) ;

    if (a_class == b_class) {
        tlv_tag_t a_value = TAG_VALUE(a->el_tag);
        tlv_tag_t b_value = TAG_VALUE(b->el_tag);

        if (a_value == b_value) {
            if (a->el_no > b->el_no) {
                return 1 ;
            }
            return 0 ;
        } else if(a_value < b_value) {
            return -1 ;
        } else {
            return 1 ;
        }
    } else if (a_class < b_class) {
        return -1 ;
    } else {
        return 1 ;
    }
}

enc_rval_t SEQUENCE_encode(type_descriptor_t *td, void *sptr, char tag_mode, tlv_tag_t tag, app_consume_bytes_f *cb, void *app_key) {
    uint8_t computed_size = 0 ;
    enc_rval_t erval ;
    char ret ;
    uint8_t edx ;

    for (edx = 0; edx < td->elements_count; edx++) {
        type_member_t *elm = &td->elements[edx] ;
        void *memb_ptr ;
        if (elm->flags & ATF_POINTER) {
            memb_ptr = *(void **)((char *)sptr + elm->memb_offset);
            if (!memb_ptr) {
                if (elm->optional) {
                    continue ;
                }
                ENCODE_FAILED ;
            }
        } else {
            memb_ptr = (void *)((char *)sptr + elm->memb_offset) ;
        }
        
        erval = ((type_encoder_f*)elm->type->encoder)(elm->type, memb_ptr, elm->tag_mode, elm->tag, 0, 0) ;
        if (erval.encoded == -1) {
            return erval ;
        }
        
        computed_size += erval.encoded ;
    }

    ret = write_tags(td, computed_size, tag_mode, 1, tag, cb, app_key) ;
    if (ret == -1) {
        ENCODE_FAILED ;
    }
    erval.encoded = computed_size + ret ;

    if (!cb) {
        ENCODED_OK(erval) ;
    }

    for(edx = 0; edx < td->elements_count; edx++) {
        type_member_t *elm = &td->elements[edx] ;
        enc_rval_t tmperval ;
        void *memb_ptr ;

        if (elm->flags & ATF_POINTER) {
            memb_ptr = *(void **)((char *)sptr + elm->memb_offset) ;
            if (!memb_ptr) {
                continue ;
            }
        } else {
            memb_ptr = (void *)((char *)sptr + elm->memb_offset) ;
        }
        
        tmperval = ((type_encoder_f*)elm->type->encoder)(elm->type, memb_ptr, elm->tag_mode, elm->tag, cb, app_key) ;
        if(tmperval.encoded == -1) {
            return tmperval ;
        }
        
        computed_size -= tmperval.encoded ;
    }

    if (computed_size != 0) {
        ENCODE_FAILED ;
    }

    ENCODED_OK(erval) ;
}

dec_rval_t SEQUENCE_decode(codec_ctx_t *opt_codec_ctx, type_descriptor_t *td, void **struct_ptr, const void *ptr, uint8_t size, char tag_mode) {
    SEQUENCE_specifics_t *specs = (SEQUENCE_specifics_t *)td->specifics ;
    type_member_t *elements = td->elements ;
    void *st = *struct_ptr ;
    struct_ctx_t *ctx ;
    tlv_tag_t tlv_tag ;
    dec_rval_t rval ;
    uint8_t consumed_myself = 0 ;
    uint8_t edx ;

    if (st == 0) {
        st = *struct_ptr = calloc(1, specs->struct_size) ;
        if (st == 0) {
            RETURN(RC_FAIL) ;
        }
    }

    
    ctx = (struct_ctx_t *)((char *)st + specs->ctx_offset) ;
    
    switch (ctx->phase) {
        case 0:
            rval = check_tags(opt_codec_ctx, td, ctx, ptr, size, tag_mode, 1, &ctx->left, 0) ;
            if (rval.code != RC_OK) {
                return rval;
            }
            
            if (ctx->left >= 0) {
                ctx->left += rval.consumed ;
            }
            ADVANCE(rval.consumed) ;
            NEXT_PHASE(ctx) ;
        
        case 1:
            for(edx = (ctx->step >> 1); edx < td->elements_count; edx++, ctx->step = (ctx->step & ~1) + 2) {
                void *memb_ptr ;
                void **memb_ptr2 ;
                char tag_len ;
                uint8_t opt_edx_end ;
                uint8_t use_bsearch ;
                uint8_t n ;

                if (ctx->step & 1) {
                    goto microphase2 ;
                }

                if (ctx->left == 0 && ((edx + elements[edx].optional == td->elements_count) || (IN_EXTENSION_GROUP(specs, edx) && specs->ext_before > td->elements_count))) {
                    PHASE_OUT(ctx) ;
                    RETURN(RC_OK) ;
                }

                tag_len = tlv_tag_fetch(ptr, LEFT, &tlv_tag) ;
                switch(tag_len) {
                    case 0:
                        if (!SIZE_VIOLATION) {
                            RETURN(RC_WMORE) ;
                        }
                    case -1:
                        RETURN(RC_FAIL) ;
                }

                if (ctx->left < 0 && ((const uint8_t *)ptr)[0] == 0) {
                    if (LEFT < 2) {
                        if (SIZE_VIOLATION) {
                            RETURN(RC_FAIL) ;
                        } else {
                            RETURN(RC_WMORE) ;
                        }
                    } else if (((const uint8_t *)ptr)[1] == 0) {
                        if ((edx + elements[edx].optional == td->elements_count) || (IN_EXTENSION_GROUP(specs, edx) && specs->ext_before > td->elements_count)) {
                            goto phase3 ;
                        }
                    }
                }

                use_bsearch = 0 ;
                opt_edx_end = edx + elements[edx].optional + 1 ;

                if (opt_edx_end > td->elements_count) {
                    opt_edx_end = td->elements_count ;
                } else if (opt_edx_end - edx > 8) {
                    opt_edx_end = edx + 8 ;
                    use_bsearch = 1 ;
                }

                for (n = edx; n < opt_edx_end; n++) {
                    if (TAGS_EQUAL(tlv_tag, elements[n].tag)) {
                        edx = n ;
                        ctx->step = 1 + 2 * edx ;
                        goto microphase2 ;
                    } else if (elements[n].flags & ATF_OPEN_TYPE) {
                        edx = n ;
                        ctx->step = 1 + 2 * edx ;
                        goto microphase2 ;
                    } else if (elements[n].tag == (tlv_tag_t)-1) {
                        use_bsearch = 1 ;
                        break ;
                    }
                }

                if (use_bsearch) {
                    const type_tag2member_t *t2m ;
                    type_tag2member_t key ;
                    key.el_tag = tlv_tag ;
                    key.el_no = edx ;
                    t2m = (const type_tag2member_t *)bsearch(&key, specs->tag2el, specs->tag2el_count, sizeof(specs->tag2el[0]), t2e_cmp) ;
                    if (t2m) {
                        const type_tag2member_t *best = 0 ;
                        const type_tag2member_t *t2m_f, *t2m_l ;
                        uint8_t edx_max = edx + elements[edx].optional ;

                        t2m_f = t2m + t2m->toff_first ;
                        t2m_l = t2m + t2m->toff_last ;
                        for (t2m = t2m_f; t2m <= t2m_l; t2m++) {
                            if (t2m->el_no > edx_max) {
                                break ;
                            }
                            
                            if (t2m->el_no < edx) {
                                continue ;
                            }
                            
                            best = t2m ;
                        }

                        if (best) {
                            edx = best->el_no ;
                            ctx->step = 1 + 2 * edx ;
                            goto microphase2 ;
                        }
                    }
        
                    n = opt_edx_end ;
                }

                if (n == opt_edx_end) {
                    if (!IN_EXTENSION_GROUP(specs, edx + elements[edx].optional)) {
                        RETURN(RC_FAIL) ;
                    } else {
                        char skip ;
                        edx += elements[edx].optional ;
                        skip = tlv_len_skip(opt_codec_ctx, TLV_CONSTRUCTED(ptr), (const char *)ptr + tag_len, LEFT - tag_len) ;
                        switch(skip) {
                            case 0: 
                                if(!SIZE_VIOLATION) {
                                    RETURN(RC_WMORE) ;
                                }
                            case -1:
                                RETURN(RC_FAIL) ;
                        }
        
                        ADVANCE(skip + tag_len) ;
                        ctx->step -= 2 ;
                        edx-- ;
                        continue ;
                    }
                }

                ctx->step |= 1 ;
            
            microphase2:

                if (elements[edx].flags & ATF_POINTER) {
                    memb_ptr2 = (void **)((char *)st + elements[edx].memb_offset) ;
                } else {
                    memb_ptr = (char *)st + elements[edx].memb_offset ;
                    memb_ptr2 = &memb_ptr ;
                }
                
                rval = ((type_decoder_f *)elements[edx].type->decoder)(opt_codec_ctx, elements[edx].type, memb_ptr2, ptr, LEFT, elements[edx].tag_mode) ;
                switch (rval.code) {
                    case RC_OK:
                        break;
                    case RC_WMORE:
                        if(!SIZE_VIOLATION) {
                            ADVANCE(rval.consumed) ;
                            RETURN(RC_WMORE) ;
                        }
                    case RC_FAIL:
                        RETURN(RC_FAIL) ;
                }
                
                ADVANCE(rval.consumed) ;
            }

    phase3:
            ctx->phase = 3 ;

        case 3:
        case 4:
            while(ctx->left) {
                char tl, ll ;

                tl = tlv_tag_fetch(ptr, LEFT, &tlv_tag) ;
                switch (tl) {
                    case 0:
                        if(!SIZE_VIOLATION) {
                            RETURN(RC_WMORE) ;
                        }
                    case -1: {
                        RETURN(RC_FAIL) ;
                    }
                }

                if (ctx->left < 0 && ((const uint8_t *)ptr)[0] == 0) {
                    if (LEFT < 2) {
                        if (SIZE_VIOLATION) {
                            RETURN(RC_FAIL) ;
                        } else {
                            RETURN(RC_WMORE) ;
                        }
                    } else if (((const uint8_t *)ptr)[1] == 0) {
                        ADVANCE(2) ;
                        ctx->left++ ;
                        ctx->phase = 4 ;
                        continue ;
                    }
                }

                if (!IN_EXTENSION_GROUP(specs, td->elements_count) || ctx->phase == 4) {
                    RETURN(RC_FAIL) ;
                }

                ll = tlv_len_skip(opt_codec_ctx, TLV_CONSTRUCTED(ptr), (const char *)ptr + tl, LEFT - tl) ;
                switch (ll) {
                    case 0:
                        if(!SIZE_VIOLATION) {
                            RETURN(RC_WMORE) ;
                        }
                    case -1: {
                        RETURN(RC_FAIL) ;
                    }
                }

                ADVANCE(tl + ll) ;
            }

            PHASE_OUT(ctx) ;
    }

    RETURN(RC_OK) ;
}

void SEQUENCE_free(type_descriptor_t *td, void *sptr, uint8_t contents_only) {
    char edx ;

    if (!td || !sptr) {
        return ;
    }

    for (edx = 0; edx < td->elements_count; edx++) {
        type_member_t *elm = &td->elements[edx] ;
        void *memb_ptr ;
        if (elm->flags & ATF_POINTER) {
            memb_ptr = *(void **)((char *)sptr + elm->memb_offset) ;
            if (memb_ptr)
                STRUCT_FREE(*elm->type, memb_ptr) ;
        } else {
            memb_ptr = (void *)((char *)sptr + elm->memb_offset) ;
            STRUCT_FREE_CONTENTS_ONLY(*elm->type, memb_ptr) ;
        }
    }

    if (!contents_only) {
        free(sptr) ;
    }
}


