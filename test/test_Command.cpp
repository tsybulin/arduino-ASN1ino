#include <Arduino.h>
#include "pt_coder.h"
#include "Command.h"

#define DEBUG

#ifdef DEBUG
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINTLN(...)
#endif

byte Command_buf[30] ;
uint8_t Command_ptr = 0 ;

char Command_consume_bytes(const void *buffer, uint8_t size, void *app_key __attribute__((unused))) {
    char *buff = (char*) buffer ;
    uint8_t i ;
    for (i = 0; i < size && Command_ptr < 30 ; i++, Command_ptr++) {
        Command_buf[Command_ptr] = buff[i] ;
    }
    
    return i ;
}

boolean testCommand(Command_t cmd) {
    boolean result = true ;
    enc_rval_t eval = encode(&DEF_Command, &cmd, Command_consume_bytes, 0) ;

    Command_t cmdN ;
    cmdN.command = 0 ;
    cmdN.args = 0 ;
    memset(&cmdN._ctx, 0, sizeof(cmdN._ctx)) ;
    void *acmd = &cmdN ;
    dec_rval_t rval = decode(0, &DEF_Command, &acmd, Command_buf, Command_ptr) ;
    DPRINTLN(rval.consumed) ;
    
    result = result
        && eval.encoded == rval.consumed
        && cmd.command == cmdN.command
        && cmd.args->arg1 == cmdN.args->arg1
        && *(cmd.args->arg2) == *(cmdN.args->arg2)
        && memcmp(cmd.args->arg3->buf, cmdN.args->arg3->buf, cmd.args->arg3->size) == 0
    ;

    ((struct_free_f *)DEF_Command.free_struct)(&DEF_Command, acmd, 0) ;

    return result ;
}

void testCommands() {
    boolean result = true ;
    
    Command_t cmd ;
    cmd.command = 0x01 ;
    cmd.args = 0 ;
    
    Arg_t arg ;
    arg.arg1 = 0x2 ;
    arg.arg2 = 0 ;
    arg.arg3 = 0 ;

    byte arg2 = 0x9a ;
    arg.arg2 = &arg2 ;

    byte tmp[] = { 0x55, 0x66 } ;
    BYTE_STRING_t arg3 ;
    arg3.size = sizeof(tmp) ;
    arg3.buf = tmp ;
    arg.arg3 = &arg3 ;
    
    cmd.args = &arg ;
    
    Command_ptr = 0 ;
    result = result && testCommand(cmd) ;
    DPRINT(F("Command ")) ;
    for (uint8_t j = 0 ; j < Command_ptr ; j++) {
        DPRINT(Command_buf[j], HEX) ; DPRINT(F(" ")) ;
    }
    DPRINTLN() ;

    Serial.println(result ? F("testCommands OK") : F("testCommands FAIL")) ;
}

