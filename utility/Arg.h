#ifndef ARG_H
#define ARG_H

#include "BYTE_STRING.h"
#include "pt_type.h"

typedef struct Arg {
    byte arg1 ;
    byte *arg2 ;
    BYTE_STRING_t *arg3 ;
    
    struct_ctx_t _ctx ;
} Arg_t ;

extern type_descriptor_t DEF_Arg ;

#endif

