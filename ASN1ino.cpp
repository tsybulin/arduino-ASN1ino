#include "ASN1ino.h"
#include <utility/pt_coder.h>

ASN1ino::ASN1ino() {
    this->ptr = 0 ;
    this->stream = &Serial ;
}

ASN1ino::ASN1ino(Stream *stream) {
    this->ptr = 0 ;
    this->stream = stream ;
}

void ASN1ino::poll() {
    if (this->stream->available()) {
        if (this->ptr < READ_BUFSIZE) {
            uint8_t size = this->stream->readBytes(this->buffer, READ_BUFSIZE - this->ptr) ;
            this->ptr += size ;
        }
    }

    if (!this->ptr) {
        return ;
    }

    Command_t *cmd = (Command_t *)calloc(1, sizeof(Command_t)) ;
    
    dec_rval_t rval = decode(0, &DEF_Command, (void**)&cmd, this->buffer, this->ptr) ;
    
    if (rval.code == RC_OK) {
        if (this->handler) {
            this->handler(this, cmd) ;
        }

        memmove(this->buffer, &this->buffer[rval.consumed], rval.consumed) ;
        this->ptr -= rval.consumed ;
    } else if (rval.code == RC_FAIL) {
        if (rval.consumed > 0) {
            this->ptr -= rval.consumed ;
        } else {
            this->ptr = 0 ;
        }
    }
    
    ((struct_free_f *)DEF_Command.free_struct)(&DEF_Command, cmd, 1) ;
}

void ASN1ino::setCommandHadler(ASN1inoCommandHandler handler) {
    this->handler = handler ;
}

char Command_consume_bytes(const void *buffer, uint8_t size, void *app_key) {
    Stream *stream = (Stream *)app_key ;
    char *buff = (char*) buffer ;
    char count = stream->write(buff, size) ;
    return count ;
}

char ASN1ino::sendCommand(byte command) {
    Command_t cmd ;
    cmd.command = command ;
    cmd.args = 0 ;
    memset(&cmd._ctx, 0, sizeof(cmd._ctx)) ;
    enc_rval_t eval = encode(&DEF_Command, &cmd, Command_consume_bytes, this->stream) ;
    return eval.encoded ;
}

char ASN1ino::sendCommand(byte command, byte arg1) {
    return this->_sendCommand(command, arg1, 0, 0, 0) ;
}

char ASN1ino::sendCommand(byte command, byte arg1, byte arg2) {
    return this->_sendCommand(command, arg1, &arg2, 0, 0) ;
}

char ASN1ino::sendCommand(byte command, byte arg1, byte arg2, byte *arg3, byte size) {
    return this->_sendCommand(command, arg1, &arg2, arg3, size) ;
}

char ASN1ino::_sendCommand(byte command, byte arg1, byte *arg2, byte *arg3, byte size) {
    Command_t cmd ;
    cmd.command = command ;

    Arg_t arg ;
    arg.arg1 = arg1 ;
    arg.arg2 = arg2 ;

    BYTE_STRING_t bs ;

    if (arg3) {
        bs.buf = arg3 ;
        bs.size = size ;
        arg.arg3 = &bs ;
    } else {
        arg.arg3 = 0 ;
    }

    cmd.args = &arg ;

    memset(&cmd._ctx, 0, sizeof(cmd._ctx)) ;
    enc_rval_t eval = encode(&DEF_Command, &cmd, Command_consume_bytes, this->stream) ;
    return eval.encoded ;
}
