#ifndef TLV_TAG_H
#define TLV_TAG_H

#include <Arduino.h>
#include <stdint.h>

typedef byte tlv_tag_t ;

#define TAG_CLASS(tag) ((tag) & 0x3)
#define TAG_VALUE(tag) ((tag) >> 2)
#define TLV_CONSTRUCTED(tagptr) (((*(const uint8_t *)tagptr) & 0x20) ? 1 : 0)
#define TAGS_EQUAL(tag1, tag2) ((tag1) == (tag2))

char tlv_tag_serialize(tlv_tag_t tag, void *bufptr, uint8_t size) ;
char tlv_tag_fetch(const void *bufptr, uint8_t size, tlv_tag_t *tag_r) ;

#endif

