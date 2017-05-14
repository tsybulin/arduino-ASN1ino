#ifndef PT_CODER_H
#define PT_CODER_H

#include "pt_application.h"
#include "tlv_tag.h"
#include "pt_type.h"

#define ENCODED_OK(rval) do { rval.structure_ptr = 0;  rval.failed_type = 0;  return rval; } while(0)
#define ENCODE_FAILED do { enc_rval_t tmp_error; tmp_error.encoded = -1; tmp_error.failed_type = td; tmp_error.structure_ptr = sptr; return tmp_error; } while(0)
#define DECODE_FAILED do { dec_rval_t tmp_error; tmp_error.code = RC_FAIL; tmp_error.consumed = 0; return tmp_error; } while(0)

struct type_descriptor_s ;
//typedef struct type_descriptor_s type_descriptor_t ;

typedef struct enc_rval_s {
    char encoded ;
    struct type_descriptor_s *failed_type ;
    void *structure_ptr ;
} enc_rval_t ;

typedef enc_rval_t (type_encoder_f)(
    struct type_descriptor_s *type_descriptor,
    void *struct_ptr,
    char tag_mode,
    tlv_tag_t tag,
    app_consume_bytes_f *consume_bytes_cb,
    void *app_key
) ;

typedef struct codec_ctx_s {
    uint8_t  max_stack_size ;
} codec_ctx_t ;

enum dec_rval_code_e {
    RC_OK,
    RC_WMORE,
    RC_FAIL
} ;

typedef struct dec_rval_s {
    enum dec_rval_code_e code ;
    uint8_t consumed ;
} dec_rval_t ;

typedef dec_rval_t (type_decoder_f)(
    struct codec_ctx_s *opt_codec_ctx,
    struct type_descriptor_s *type_descriptor,
    void **struct_ptr, const void *buf_ptr, uint8_t size,
    char tag_mode
) ;

typedef struct PRIMITIVE_TYPE_s {
        uint8_t *buf ;
        uint8_t size ;
} PRIMITIVE_TYPE_t ;

char write_tags(
    struct type_descriptor_s *type_descriptor,
    uint8_t struct_length,
    char tag_mode,
    uint8_t last_tag_form,
    tlv_tag_t tag,
    app_consume_bytes_f *consume_bytes_cb,
    void *app_key
) ;

typedef void (struct_free_f)(struct type_descriptor_s *type_descriptor, void *struct_ptr, uint8_t free_contents_only) ;
struct_free_f primitive_free  ;

type_encoder_f encode_primitive ;
enc_rval_t encode(struct type_descriptor_s *type_descriptor, void *struct_ptr, app_consume_bytes_f *consume_bytes, void *app_key) ;

type_decoder_f decode_primitive ;
dec_rval_t decode(codec_ctx_t *opt_codec_ctx, type_descriptor_t *type_descriptor, void **struct_ptr, const void *ptr, uint8_t size) ;

dec_rval_t check_tags(
    codec_ctx_t *opt_codec_ctx, struct type_descriptor_s *td, struct_ctx_t *opt_ctx,
    const void *ptr, uint8_t size, char tag_mode, char last_tag_form,
    char *last_length, uint8_t *opt_tlv_form
) ;

#endif

