#include <Arduino.h>
#include "pt_coder.h"
#include "BYTE.h"

//#define DEBUG

#ifdef DEBUG
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINTLN(...)
#endif

byte BYTE_buf[50] ;
uint8_t BYTE_ptr = 0 ;

char BYTE_consume_bytes(const void *buffer, uint8_t size, void *app_key __attribute__((unused))) {
    char *buff = (char*) buffer ;
    uint8_t i ;
    for (i = 0; i < size && BYTE_ptr < 50 ; i++, BYTE_ptr++) {
        BYTE_buf[BYTE_ptr] = buff[i] ;
    }
    
    return i ;
}

boolean testBYTE(byte b) {
    //enc_rval_t eval = 
    encode(&DEF_BYTE, &b, BYTE_consume_bytes, 0) ;
    byte bt ;
    void *bbt = &bt ;
    //dec_rval_t rval = 
    decode(0, &DEF_BYTE, &bbt, BYTE_buf, BYTE_ptr) ;
    
    return b == bt ;
}

void testBYTEs() {
    boolean result = true ;

    byte b = 0 ;
    for (int i = 0; i <= 0xff; i++, b++) {
        result = result && testBYTE(b) ;

        DPRINT(b, HEX) ; DPRINT(F(" : ")) ; 
        for (uint8_t j = 0 ; j < BYTE_ptr ; j++) {
            DPRINT(BYTE_buf[j], HEX) ; DPRINT(F(" ")) ;
        }
        DPRINTLN() ;
        
        BYTE_ptr = 0 ;
    }
    
    Serial.println(result ? F("testBYTEs OK") : F("testBYTEs FAIL")) ;
}

