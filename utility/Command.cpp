#include "Command.h"
#include "Arg.h"
#include "BYTE.h"
#include "pt_coder.h"
#include "SEQUENCE.h"

static const tlv_tag_t DEF_Command_tags[] = { (TAG_CLASS_UNIVERSAL | (4 << 2)) } ;

static type_member_t MBR_Command[] {
    {
        ATF_NOFLAGS,
        0,
        offsetof(struct Command, command),
        (TAG_CLASS_CONTEXT | (4 << 2)),
        +1,
        &DEF_BYTE,
        0
    },
    {
        ATF_POINTER,
        1,
        offsetof(struct Command, args),
        (TAG_CLASS_CONTEXT | (5 << 2)),
        +1,
        &DEF_Arg,
        0
    }
} ;

static const type_tag2member_t MAP_Command_tag2el[] = {
    { (TAG_CLASS_CONTEXT | (4 << 2)), 0, 0, 0 },
    { (TAG_CLASS_CONTEXT | (5 << 2)), 1, 0, 0 }
} ;

static SEQUENCE_specifics_t SPC_Command_specs = {
    sizeof(struct Command),
    offsetof(struct Command, _ctx),
    MAP_Command_tag2el,
    2,
    -1,
    -1
} ;

extern type_encoder_f SEQUENCE_encode ;
extern type_decoder_f SEQUENCE_decode ;
extern struct_free_f SEQUENCE_free ;

type_descriptor_t DEF_Command = {
    (anyfunc *) SEQUENCE_free,
    (anyfunc *) SEQUENCE_decode,
    (anyfunc *) SEQUENCE_encode,
    DEF_Command_tags,
    1,
    DEF_Command_tags,
    1,
    2,
    MBR_Command,
    &SPC_Command_specs
} ;

