#include <Arduino.h>
#include "tlv_len.h"

#undef TEST_DEBUG

boolean testLen(tlv_len_t len) {
    const byte BUFFER_LENGTH = 2 ;
    byte buffer[BUFFER_LENGTH] ;

    char sz = tlv_len_serialize(len, buffer, BUFFER_LENGTH) ;

#ifdef TEST_DEBUG
    Serial.print(len, HEX) ; Serial.print(F(" :")) ;
    for (byte i = 0; i < sz; i++) {
         Serial.print(F(" ")) ; Serial.print(buffer[i], HEX) ;
    }
    Serial.println() ;
#endif

    tlv_len_t l = 0 ;
    sz = tlv_len_fetch(true, buffer, sz, &l) ;

    if (len == l) {
        return true ;
    } else {
        Serial.print(F("testLen ")) ; Serial.print(len, HEX) ; Serial.println(F(" FAIL")) ;
        return false ;
    }
}

void testLens() {
    boolean result = true ;
    tlv_len_t l = 0 ;
    for (int i = 0 ; i <= 0xff; i++, l++) {
        result = result && testLen(l) ;
    }
    Serial.println(result ? F("testLen OK") : F("testLen FAIL")) ;
}

