#ifndef PT_TYPE_H
#define PT_TYPE_H

#include <Arduino.h>
#include <stdint.h>
#include "tlv_len.h"
#include "tlv_tag.h"

enum tag_class {
    TAG_CLASS_UNIVERSAL   = B00,
    TAG_CLASS_APPLICATION = B01,
    TAG_CLASS_CONTEXT     = B10,
    TAG_CLASS_PRIVATE     = B11
} ;

#define DEFAULT_STACK_MAX (255)

typedef void (anyfunc)(void) ;

struct type_member_s ;

typedef struct type_descriptor_s {
    anyfunc *free_struct ;
    anyfunc *decoder ;
    anyfunc *encoder ;
    const tlv_tag_t *tags ;
    uint8_t tags_count ;
    const tlv_tag_t *all_tags ;
    uint8_t all_tags_count ;
    uint8_t elements_count ;
    struct type_member_s *elements ;
    const void *specifics ;
} type_descriptor_t ;

typedef struct struct_ctx_s {
    uint8_t phase ;
    uint8_t step ;
    uint8_t context ;
    void *ptr ;
    char left ;
} struct_ctx_t ;

enum type_flags_e {
    ATF_NOFLAGS,
    ATF_POINTER   = 0x01,
    ATF_OPEN_TYPE = 0x02
} ;

typedef struct type_member_s {
    enum type_flags_e flags ;
    uint8_t optional ;
    uint8_t memb_offset ;
    tlv_tag_t tag ;
    char tag_mode ;
    type_descriptor_t *type ;
    uint8_t (*default_value)(int setval, void **sptr) ;
} type_member_t ;

typedef struct type_tag2member_s {
    tlv_tag_t el_tag ;
    uint8_t el_no ;
    uint8_t toff_first ;
    uint8_t toff_last ;
} type_tag2member_t ;

#endif

