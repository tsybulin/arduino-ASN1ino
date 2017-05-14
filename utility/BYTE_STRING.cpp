#include "tlv_tag.h"
#include "pt_type.h"
#include "pt_coder.h"

static const tlv_tag_t DEF_BYTE_STRING_tags[] = { (TAG_CLASS_APPLICATION | (2 << 2)) } ;

type_descriptor_t DEF_BYTE_STRING = {
    (anyfunc *) primitive_free,
    (anyfunc *) decode_primitive,
    (anyfunc *) encode_primitive,
    DEF_BYTE_STRING_tags,
    1,
    DEF_BYTE_STRING_tags,
    1,
    0,
    0,
    0
} ;

