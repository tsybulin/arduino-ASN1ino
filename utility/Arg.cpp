#include "Arg.h"
#include "BYTE.h"
#include "BYTE_STRING.h"
#include "pt_coder.h"
#include "SEQUENCE.h"

static const tlv_tag_t DEF_Arg_tags[] = { (TAG_CLASS_UNIVERSAL | (3 << 2)) } ;

static type_member_t MBR_Arg[] {
    {
        ATF_NOFLAGS,
        0,
        offsetof(struct Arg, arg1),
        (TAG_CLASS_CONTEXT | (1 << 2)),
        +1,
        &DEF_BYTE,
        0
    },
    {
        ATF_POINTER,
        2,
        offsetof(struct Arg, arg2),
        (TAG_CLASS_CONTEXT | (2 << 2)),
        +1,
        &DEF_BYTE,
        0
    },
    {
        ATF_POINTER,
        1,
        offsetof(struct Arg, arg3),
        (TAG_CLASS_CONTEXT | (3 << 2)),
        +1,
        &DEF_BYTE_STRING,
        0
    }
} ;

static const type_tag2member_t MAP_Arg_tag2el[] = {
    { (TAG_CLASS_CONTEXT | (1 << 2)), 0, 0, 0 },
    { (TAG_CLASS_CONTEXT | (2 << 2)), 1, 0, 0 },
    { (TAG_CLASS_CONTEXT | (3 << 2)), 2, 0, 0 }
} ;

static SEQUENCE_specifics_t SPC_Arg_specs = {
    sizeof(struct Arg),
    offsetof(struct Arg, _ctx),
    MAP_Arg_tag2el,
    3,
    -1,
    -1
} ;

extern type_encoder_f SEQUENCE_encode ;
extern type_decoder_f SEQUENCE_decode ;
extern struct_free_f SEQUENCE_free ;

type_descriptor_t DEF_Arg = {
    (anyfunc *) SEQUENCE_free,
    (anyfunc *) SEQUENCE_decode,
    (anyfunc *) SEQUENCE_encode,
    DEF_Arg_tags,
    1,
    DEF_Arg_tags,
    1,
    3,
    MBR_Arg,
    &SPC_Arg_specs
} ;

