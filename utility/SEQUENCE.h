#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "pt_type.h"

typedef const struct SEQUENCE_specifics_s {
    uint8_t struct_size ;
    uint8_t ctx_offset ;
    const type_tag2member_t *tag2el ;
    uint8_t tag2el_count ;
    char ext_after ;
    char ext_before ;
} SEQUENCE_specifics_t ;

#endif


