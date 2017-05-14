#include <Arduino.h>
#include "pt_coder.h"
#include "BYTE_STRING.h"

// #define DEBUG

#ifdef DEBUG
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINTLN(...)
#endif

byte BYTE_STRING_buf[50] ;
uint8_t BYTE_STRING_ptr = 0 ;

char BYTE_STRING_consume_bytes(const void *buffer, uint8_t size, void *app_key __attribute__((unused))) {
    char *buff = (char*) buffer ;
    uint8_t i ;
    for (i = 0; i < size && BYTE_STRING_ptr < 50 ; i++, BYTE_STRING_ptr++) {
        BYTE_STRING_buf[BYTE_STRING_ptr] = buff[i] ;
    }
    
    return i ;
}

boolean testBYTE_STRING(byte *b, uint8_t len) {
    BYTE_STRING_t bs1 ;
    bs1.size = len ;
    bs1.buf = b ;
    encode(&DEF_BYTE_STRING, &bs1, BYTE_STRING_consume_bytes, 0) ;
    BYTE_STRING_t bs2 ;
    bs2.size = 0 ;
    void *bbs2 = &bs2 ;
    decode(0, &DEF_BYTE_STRING, &bbs2, BYTE_STRING_buf, BYTE_STRING_ptr) ;
    boolean result =  bs1.size == bs2.size && memcmp(bs1.buf, bs2.buf, bs1.size) == 0 ;

    ((struct_free_f *)DEF_BYTE_STRING.free_struct)(&DEF_BYTE_STRING, &bs2, 0) ;

    return result ;
}

void testBYTE_STRINGs() {
    boolean result = true ;
    byte b[] = { 0, 1, 2, 3, 4, 5, 6 } ;
    result = result && testBYTE_STRING(&b[0], sizeof(b)) ;

        //DPRINT(b, HEX) ; DPRINT(F(" : ")) ; 
        for (uint8_t j = 0 ; j < BYTE_STRING_ptr ; j++) {
            DPRINT(BYTE_STRING_buf[j], HEX) ; DPRINT(F(" ")) ;
        }
        DPRINTLN() ;

    Serial.println(result ? F("testBYTE_STRINGs OK") : F("testBYTE_STRINGs FAIL")) ;
}

