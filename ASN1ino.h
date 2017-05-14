#ifndef ASN1INO_H
#define ASN1INO_H

#include <Arduino.h>
#include <utility/Command.h>

#define READ_BUFSIZE 50

class ASN1ino ;

typedef void (*ASN1inoCommandHandler)(ASN1ino *asn1ino, Command_t *cmd) ;

class ASN1ino {
    public:
        ASN1ino() ;
        ASN1ino(Stream *stream) ;
        void poll() ;
        void setCommandHadler(ASN1inoCommandHandler handler) ;
        char sendCommand(byte command) ;
        char sendCommand(byte command, byte arg1) ;
        char sendCommand(byte command, byte arg1, byte arg2) ;
        char sendCommand(byte command, byte arg1, byte arg2, byte *arg3, byte size) ;
    private:
        char _sendCommand(byte command, byte arg1, byte *arg2, byte *arg3, byte size) ;
        ASN1inoCommandHandler handler ;
        byte buffer[READ_BUFSIZE + 1] ;
        uint8_t ptr ;
        Stream *stream ;
} ;

#endif

