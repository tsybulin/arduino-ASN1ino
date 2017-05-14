#include <Arduino.h>
#include "tlv_tag.h"

// #define TEST_DEBUG

boolean testTag(tlv_tag_t tag) {
    const byte BUFFER_LENGTH = 2 ;
    byte buffer[BUFFER_LENGTH] ;

    char sz = tlv_tag_serialize(tag, buffer, BUFFER_LENGTH) ;

#ifdef TEST_DEBUG
    Serial.print(tag, HEX) ; Serial.print(F(" :")) ;
    for (byte i = 0; i < sz; i++) {
         Serial.print(F(" ")) ; Serial.print(buffer[i], HEX) ;
    }
    Serial.println() ;
#endif    
    tlv_tag_t t = 0 ;
    sz = tlv_tag_fetch(buffer, sz, &t) ;

    if (tag == t) {
        return true ;
    } else {
        Serial.print(F("testTag ")) ; Serial.print(tag, HEX) ; Serial.println(F(" FAIL")) ;
        return false ;
    }
}

void testTags() {
    boolean result = true ;
    tlv_tag_t t = 0 ;
    for (int i = 0 ; i <= 0xff; i++, t++) {
        result = result && testTag(t) ;
    }
    Serial.println(result ? F("testTag OK") : F("testTag FAIL")) ;
}

