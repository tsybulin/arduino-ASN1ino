#ifndef PT_APPLICATION_H
#define PT_APPLICATION_H

#include <Arduino.h>
#include <stdint.h>

typedef char (app_consume_bytes_f)(const void *buffer, uint8_t size, void *application_specific_key) ;

#endif

