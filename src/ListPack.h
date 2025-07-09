/*
                                Moto(Redis str)

Index:    0  1  2  3    4  5        6        7  8  9 10 11     12          13
         +-----------+--------+----------+----------------+-----------+----------+
Value:   | 14 (u32)  | 1 (u16)| 0 (type) | 'h''e''l''l''o' |  6 (u8)   | 255 (u8) |
         +-----------+--------+----------+----------------+-----------+----------+
Meaning: | Total Bytes| Num Elem | <-- Entry 1: <encoding+data> --> | <backlen> |   EOF    |



entry1   entry2.backlen= sizeof(entry1) as for backward traversel
so a list pack can have max 256 elements and can go upto 512/memory execdes 
if 512 reached then split them into two . This is managed by quicklist 

*/
#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <algorithm>

// encoding reference from redis 
class ListPack {
private:
    char* buffer=nullptr; // start point ref
    size_t last_entry_size=0;
    size_t capacity=0;
    int entries=0;

    static const size_t TOTAL_BYTES_OFFSET=0;
    static const size_t NUM_ELEMENTS_OFFSET=4;
    static const size_t HEADER_SIZE=6;
    static const uint8_t EOF_B=255;

    // --- Encoding Constants (from redis) ---
    static const uint8_t BUFFER_ENCODING_7BIT_UINT = 0x00;
    static const uint8_t BUFFER_ENCODING_13BIT_INT = 0xC0;
    static const uint8_t BUFFER_ENCODING_16BIT_INT = 0xF1;
    static const uint8_t BUFFER_ENCODING_24BIT_INT = 0xF2;
    static const uint8_t BUFFER_ENCODING_32BIT_INT = 0xF3;
    static const uint8_t BUFFER_ENCODING_64BIT_INT = 0xF4;

    static const uint8_t BUFFER_ENCODING_6BIT_STR = 0x80;
    static const uint8_t BUFFER_ENCODING_12BIT_STR = 0xE0;
    static const uint8_t BUFFER_ENCODING_32BIT_STR = 0xF0;

    static const uint8_t BUFFER_ENCODING_MASK = 0xC0;
    static const uint8_t BUFFER_ENCODING_INT_MASK = 0xF0;

    void set_total_bytes(uint32_t size);

    void set_num_elements(uint16_t count);

    bool check_capacity(size_t required);

    size_t get_encoded_content_len(const std::string& s) const;

    // Writes the <encoding+data> part to the buffer 'p' for a given string 's'.
    void buffer_encode_element(char* p, const std::string& s);

    // to get the byte len of elemnet
    size_t buffer_read_len(const char** p) const;

    // getiing the length of elemnte in list by encoding 
    size_t get_element_content_len(const char* p) const;

    // Decodes the size of the backlen field by looking at the last byte
    size_t get_decode_backlen_size(const char* p) const;

    // write the bcklen at pointer p 
    void buffer_encode_backlen(char* p, size_t len);
    
    size_t get_encoded_backlen_size(size_t len) const;

    // Decode 
    size_t buffer_decode_backlen(const char* p);

    // Give a full entry lenght content+backlen
    size_t get_full_entry_len(const char* p);

    char* replace_bytes(char* pos, const std::string& str, bool is_inserting);

    char* erase_bytes(char* pos);

public:

    ListPack();

    uint16_t get_num_elements();

    uint32_t get_total_bytes();

    char* insert(char* ptr, const std::string &str);
    
    char* erase(char* p);

    std::optional<std::string> pop_front();
    std::optional<std::string> pop_back();

    bool push_back(const std::string& value);
    bool push_front(const std::string& value);

    char* find_entry_from_head(int index);
    std::optional<std::string> get_string(const char* p) const;
    std::vector<std::string> get_range(size_t st ,size_t ed);
    
    ~ListPack() {
        free(buffer);
    };
};
