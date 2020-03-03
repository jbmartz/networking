//
// Created by Justin Martz on 2019-10-24.
//

#ifndef UDP_MINI_PROJ1_CF_SERVER_H
#define UDP_MINI_PROJ1_CF_SERVER_H


#include <stdint.h>

enum DataType {
    TEXT_DATA = 200,
    NUMBER_DATA,
    OTHER
};

struct BaseHeader {
    uint16_t DataType;
    uint16_t DataSize;
} __attribute__((__packed__));

struct NumberData {
    struct BaseHeader hdr;
    uint32_t number_data;
} __attribute__((__packed__));

struct TextData {
    struct BaseHeader hdr;
    uint32_t text_data_length;
} __attribute__((__packed__)); /* Text data follows this in memory */

#endif //IN_CLASS_UDP_EXAMPLE_IN_CLASS_UDP_EXAMPLE_H
