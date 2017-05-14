#include <Arduino.h>
#include "pt_coder.h"
#include "Arg.h"

#undef DEBUG

#ifdef DEBUG
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINTLN(...)
#endif

byte Arg_buf[50] ;
uint8_t Arg_ptr = 0 ;

char Arg_consume_bytes(const void *buffer, uint8_t size, void *app_key __attribute__((unused))) {
    char *buff = (char*) buffer ;
    uint8_t i ;
    for (i = 0; i < size && Arg_ptr < sizeof(Arg_buf) ; i++, Arg_ptr++) {
        Arg_buf[Arg_ptr] = buff[i] ;
    }
    
    return i ;
}

boolean testArg(Arg_t arg) {
    enc_rval_t eval = encode(&DEF_Arg, &arg, Arg_consume_bytes, 0) ;

    Arg_t argN ;
    argN.arg1 = 0 ;
    argN.arg2 = 0 ;
    argN.arg3 = 0 ;

    memset(&argN._ctx, 0, sizeof(argN._ctx)) ;

    void *aarg = &argN ;

    dec_rval_t rval = decode(0, &DEF_Arg, &aarg, Arg_buf, Arg_ptr) ;
    DPRINTLN(rval.consumed) ;
    
    boolean result = true ;
    result = result && eval.encoded == rval.consumed &&
        arg.arg1 == argN.arg1 &&
        *(arg.arg2) == *(argN.arg2) &&
        arg.arg3->size == argN.arg3->size &&
        memcmp(arg.arg3->buf, argN.arg3->buf, arg.arg3->size) == 0
        ;

    ((struct_free_f *)DEF_Arg.free_struct)(&DEF_Arg, aarg, 0) ;
    
    return result ;
}

void testArgs() {
    Arg_t arg ;
    arg.arg1 = 0x2 ;

    arg.arg2 = 0 ;
    byte arg2 = 0x9a ;
    arg.arg2 = &arg2 ;

    arg.arg3 = 0 ;
    byte tmp[] = { 'A', 'r', 'd', 'u', 'i', 'n', 'o' } ;
    BYTE_STRING_t arg3 ;
    arg3.size = sizeof(tmp) ;
    arg3.buf = tmp ;
    arg.arg3 = &arg3 ;
    
    Arg_ptr = 0 ;
    boolean result = testArg(arg) ;
    DPRINT(F("Arg ")) ;
    for (uint8_t j = 0 ; j < Arg_ptr ; j++) {
        DPRINT(Arg_buf[j], HEX) ; DPRINT(F(" ")) ;
    }
    DPRINTLN() ;

    Serial.println(result ? F("testArgs OK") : F("testArgs FAIL")) ;
}

