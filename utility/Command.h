#ifndef COMMAND_H
#define COMMAND_H

#include "pt_type.h"
#include "Arg.h"

typedef struct Command {
    byte command ;
    Arg_t *args ;
    
    struct_ctx_t _ctx ;
} Command_t ;

extern type_descriptor_t DEF_Command ;


#endif

