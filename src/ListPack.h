#pragma once 

#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>



/*
                                Moto

Index:    0  1  2  3    4  5        6        7  8  9 10 11     12          13
         +-----------+--------+----------+----------------+-----------+----------+
Value:   | 14 (u32)  | 1 (u16)| 0 (type) | 'h''e''l''l''o' |  6 (u8)   | 255 (u8) |
         +-----------+--------+----------+----------------+-----------+----------+
Meaning: | Total Bytes| Num Elem | <-- Entry 1: <encoding+data> --> | <backlen> |   EOF    |

*/

/*
so a list pack can have max 256 elements and can go upto 512/memory execdes 
if 512 reached then split them into two .

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
#include <algorithm> // For std::reverse




class ListPack
{
private:
    char* buffer=nullptr; // start point ref
    size_t capacity=0;
    static const size_t TOTAL_BYTES_OFFSET=0;
    static const size_t NUM_ELEMENTS_OFFSET=4;
    static const size_t HEADER_SIZE=6;
    static const uint8_t EOF_B=255;

    // to set the totalbytes in listpack
    void set_total_bytes(uint32_t& size) {
        memcpy(buffer+TOTAL_BYTES_OFFSET,&size,sizeof(size));
    }

    // to set the total nuumber of the elements 
    void set_num_elements(uint16_t&count){
        memcpy(buffer+NUM_ELEMENTS_OFFSET,&count,sizeof(count));
    }

    // to reallocate the entire memory of listpack as it required 
    bool check_capacity(size_t required) {
        if(capacity>=required) return true;
        size_t new_capacity= (capacity==0)?32:2*capacity;
        if(new_capacity<required) new_capacity=required;
        
        char* new_buff= static_cast<char*>(realloc(buffer,new_capacity));
        buffer=new_buff;
        capacity=new_capacity;
        return true;
    }

    char* replace_bytes(char * st,const std::string& str) {
        
    }

public:
    ListPack(/* args */);
    ~ListPack();
};






// A faithful C++ replica of the Redis listpack data structure.
// This version includes variable-length encodings and correct backlen updates.
class Listpack {
private:
    char* lp_ = nullptr;
    size_t capacity_ = 0;

    // --- Header and Metadata Constants ---
    static const size_t TOTAL_BYTES_OFFSET = 0;
    static const size_t NUM_ELEMENTS_OFFSET = 4;
    static const size_t HEADER_SIZE = 6;
    static const uint8_t EOF_BYTE = 0xFF;

    // --- Encoding Constants (from listpack.c) ---
    // 7-bit unsigned integer
    static const uint8_t LP_ENCODING_7BIT_UINT = 0x00;
    // 6-bit string length
    static const uint8_t LP_ENCODING_6BIT_STR = 0x80;
    // 13-bit signed integer
    static const uint8_t LP_ENCODING_13BIT_INT = 0xC0;
    // 12-bit string length
    static const uint8_t LP_ENCODING_12BIT_STR = 0xE0;
    // 16-bit signed integer
    static const uint8_t LP_ENCODING_16BIT_INT = 0xF1;
    // 24-bit signed integer
    static const uint8_t LP_ENCODING_24BIT_INT = 0xF2;
    // 32-bit signed integer
    static const uint8_t LP_ENCODING_32BIT_INT = 0xF3;
    // 64-bit signed integer
    static const uint8_t LP_ENCODING_64BIT_INT = 0xF4;
    // 32-bit string length
    static const uint8_t LP_ENCODING_32BIT_STR = 0xF0;

    static const uint8_t LP_ENCODING_MASK = 0xC0;
    static const uint8_t LP_ENCODING_INT_MASK = 0xF0;

    // --- Private Helper Methods for Memory and Layout ---

    void set_total_bytes(uint32_t size) noexcept {
        memcpy(lp_ + TOTAL_BYTES_OFFSET, &size, sizeof(size));
    }

    void set_num_elements(uint16_t count) noexcept {
        memcpy(lp_ + NUM_ELEMENTS_OFFSET, &count, sizeof(count));
    }

    bool ensure_capacity(size_t required_size) {
        if (capacity_ >= required_size) return true;
        size_t new_capacity = capacity_ > 0 ? capacity_ * 2 : 32;
        if (new_capacity < required_size) new_capacity = required_size;
        
        char* new_buffer = static_cast<char*>(realloc(lp_, new_capacity));
        if (!new_buffer) return false;
        
        lp_ = new_buffer;
        capacity_ = new_capacity;
        return true;
    }

    // --- Core Private Helper: The Generic Replace Function ---
    // This function is the heart of insert and erase. It replaces a range of 'len' bytes
    // at position 'p' with a new element 'ele'.
    char* lp_replace(char* p, const std::string& ele) {
        size_t old_entry_len = (p[0] == EOF_BYTE) ? 0 : lp_element_total_len(p);

        // 1. Calculate new entry size
        size_t new_ele_len = lp_get_encoded_len(ele);
        size_t new_backlen_len = lp_get_backlen_size(new_ele_len);
        size_t new_entry_len = new_ele_len + new_backlen_len;

        // 2. Find the element that follows the one being replaced.
        char* next_p = p + old_entry_len;
        size_t next_entry_backlen_delta = 0;
        if (next_p[0] != EOF_BYTE) {
            size_t old_next_backlen_size = lp_decode_backlen_size(next_p);
            size_t new_next_backlen_size = lp_get_backlen_size(new_entry_len);
            next_entry_backlen_delta = new_next_backlen_size - old_next_backlen_size;
        }

        // 3. Calculate memory shifts
        int64_t len_diff = new_entry_len - old_entry_len;
        size_t total_shift = len_diff + next_entry_backlen_delta;
        size_t old_total_bytes = get_total_bytes();
        size_t new_total_bytes = old_total_bytes + total_shift;

        if (!ensure_capacity(new_total_bytes)) return nullptr;

        // Correct pointers in case realloc moved the buffer
        p = lp_ + (p - lp_);
        next_p = p + old_entry_len;

        // 4. Make room by shifting memory
        if (total_shift != 0) {
            memmove(next_p + total_shift, next_p, old_total_bytes - (next_p - lp_));
        }

        // 5. Write the new entry and its backlen
        lp_encode_element(p, ele);
        lp_encode_backlen(p + new_ele_len, new_ele_len);
        char* new_p = p;

        // 6. Update the backlen of the next entry if it exists
        if (next_p[0] != EOF_BYTE) {
            char* next_after_shift = new_p + new_entry_len;
            lp_encode_backlen(next_after_shift + lp_element_content_len(next_after_shift), new_entry_len);
        }

        // 7. Update header
        set_total_bytes(new_total_bytes);
        if (old_entry_len == 0) { // This was an insert
            set_num_elements(get_num_elements() + 1);
        }

        return new_p;
    }
    
    // --- Private Encoding/Decoding Helpers ---
    
    // Simplified encoding logic for strings
    size_t lp_get_encoded_len(const std::string& s) const {
        return 1 + s.length();
    }
    void lp_encode_element(char* p, const std::string& s) {
        p[0] = LP_ENCODING_6BIT_STR | s.length();
        memcpy(p + 1, s.c_str(), s.length());
    }

    size_t lp_get_backlen_size(size_t len) const {
        if (len < 128) return 1;
        if (len < 16384) return 2;
        if (len < 2097152) return 3;
        if (len < 268435456) return 4;
        return 5;
    }
    
    void lp_encode_backlen(char* p, size_t len) {
        if (len < 128) {
            p[0] = len;
        } else if (len < 16384) {
            p[0] = (len >> 8) | 0x80;
            p[1] = len & 0xFF;
        } else { // Simplified for brevity
            p[0] = (len >> 24) | 0xE0;
            p[1] = (len >> 16) & 0xFF;
            p[2] = (len >> 8) & 0xFF;
            p[3] = len & 0xFF;
        }
    }

    size_t lp_decode_backlen(const char* p) const {
        size_t val = 0;
        size_t shift = 0;
        const unsigned char* up = reinterpret_cast<const unsigned char*>(p);
        do {
            val |= (size_t)(up[0] & 127) << shift;
            if (!(up[0] & 128)) break;
            shift += 7;
            up--;
        } while(true);
        return val;
    }
    
    // Decodes the size of the backlen field by looking at the last byte
    size_t lp_decode_backlen_size(const char* p) const {
        size_t len = 1;
        // The backlen is at the end of the element.
        const char* end_of_element = p + lp_element_total_len(p) - 1;
        const unsigned char* up = reinterpret_cast<const unsigned char*>(end_of_element);
        while(*up & 0x80) {
            len++;
            up--;
        }
        return len;
    }

    size_t lp_element_content_len(const char* p) const {
        uint8_t enc = p[0];
        if ((enc & LP_ENCODING_MASK) == LP_ENCODING_6BIT_STR) {
            return 1 + (enc & 0x3F);
        }
        // ... add other encoding types here
        return 0; // Should not happen in this simplified impl
    }
    
    size_t lp_element_total_len(const char* p) const {
        if (p[0] == EOF_BYTE) return 1;
        size_t content_len = lp_element_content_len(p);
        const char* backlen_ptr = p + content_len;
        size_t backlen_size = 1;
        while(backlen_ptr[backlen_size-1] & 0x80) {
            backlen_size++;
        }
        return content_len + backlen_size;
    }

public:
    Listpack() {
        size_t initial_size = HEADER_SIZE + sizeof(EOF_BYTE);
        lp_ = static_cast<char*>(malloc(initial_size));
        if (!lp_) throw std::bad_alloc();
        capacity_ = initial_size;
        set_total_bytes(initial_size);
        set_num_elements(0);
        lp_[HEADER_SIZE] = EOF_BYTE;
    }

    ~Listpack() {
        free(lp_);
    }

    Listpack(const Listpack&) = delete;
    Listpack& operator=(const Listpack&) = delete;
    Listpack(Listpack&&) = delete;
    Listpack& operator=(Listpack&&) = delete;

    // --- Public API ---

    uint16_t get_num_elements() const {
        uint16_t count;
        memcpy(&count, lp_ + NUM_ELEMENTS_OFFSET, sizeof(count));
        return count;
    }

    uint32_t get_total_bytes() const {
        uint32_t size;
        memcpy(&size, lp_ + TOTAL_BYTES_OFFSET, sizeof(size));
        return size;
    }

    char* insert(char* p, const std::string& value) {
        return lp_replace(p, value);
    }

    char* erase(char* p) {
        if (p[0] == EOF_BYTE || get_num_elements() == 0) return p;
        
        size_t old_entry_len = lp_element_total_len(p);
        char* next_p = p + old_entry_len;
        size_t next_backlen_delta = 0;
        
        if (next_p[0] != EOF_BYTE) {
            // New backlen will be that of the element before p
            size_t prev_len = lp_decode_backlen(p + old_entry_len-1);
            size_t old_next_backlen_size = lp_decode_backlen_size(next_p);
            size_t new_next_backlen_size = lp_get_backlen_size(prev_len);
            next_backlen_delta = new_next_backlen_size - old_next_backlen_size;
        }

        int64_t total_shift = -(int64_t)old_entry_len + next_backlen_delta;
        size_t old_total_bytes = get_total_bytes();
        size_t new_total_bytes = old_total_bytes + total_shift;

        memmove(p + total_shift, next_p, old_total_bytes - (next_p - lp_));
        
        if (next_p[0] != EOF_BYTE) {
             size_t prev_len = lp_decode_backlen(p + old_entry_len-1);
             lp_encode_backlen(p + total_shift + lp_element_content_len(p + total_shift), prev_len);
        }

        set_total_bytes(new_total_bytes);
        set_num_elements(get_num_elements() - 1);
        
        return p;
    }
    
    char* find_entry_from_head(int index) const {
        if (index < 0 || index >= get_num_elements()) return nullptr;
        char* p = lp_ + HEADER_SIZE;
        for (int i = 0; i < index; ++i) {
            p += lp_element_total_len(p);
        }
        return p;
    }

    char* find_entry_from_tail(int index) const {
        int num_elements = get_num_elements();
        if (index < 0 || index >= num_elements) return nullptr;
        
        char* p = lp_ + get_total_bytes() - 1; // Start at EOF
        for(int i = 0; i <= index; ++i) {
             size_t backlen = lp_decode_backlen(p - 1);
             p -= backlen;
        }
        return p;
    }
    
    bool push_back(const std::string& value) {
        char* eof_ptr = lp_ + get_total_bytes() - 1;
        return insert(eof_ptr, value) != nullptr;
    }

    bool push_front(const std::string& value) {
        char* first_ptr = lp_ + HEADER_SIZE;
        return insert(first_ptr, value) != nullptr;
    }
    
    std::optional<std::string> get_string(const char* p) const {
        if (!p || p[0] == EOF_BYTE) return std::nullopt;
        
        uint8_t enc = p[0];
        if ((enc & LP_ENCODING_MASK) == LP_ENCODING_6BIT_STR) {
            uint8_t len = enc & 0x3F;
            return std::string(p + 1, len);
        }
        // Could add other string decoders here
        return std::nullopt;
    }

    void debug_print() const {
        std::cout << "--- Listpack Debug ---" << std::endl;
        std::cout << "Total Bytes: " << get_total_bytes() 
                  << ", Num Elements: " << get_num_elements()
                  << ", Capacity: " << capacity_ << std::endl;
        
        char* p = lp_ + HEADER_SIZE;
        int i = 0;
        while(p[0] != EOF_BYTE) {
            std::cout << "Entry " << i++ << " at offset " << (p - lp_) << ": ";
            auto val = get_string(p);
            if (val) {
                std::cout << "'" << *val << "'";
            } else {
                std::cout << "[non-string value]";
            }
            size_t total_len = lp_element_total_len(p);
            size_t content_len = lp_element_content_len(p);
            size_t backlen_size = total_len - content_len;
            size_t backlen = lp_decode_backlen(p + total_len - 1);
            std::cout << " (total_len=" << total_len << ", content_len=" << content_len 
                      << ", backlen_size=" << backlen_size << ", backlen_val=" << backlen << ")" << std::endl;
            p += total_len;
        }
        std::cout << "EOF at offset " << (p - lp_) << std::endl;
        std::cout << "----------------------" << std::endl;
    }
};



// // --- Example Usage ---
// int main() {
//     Listpack lp;
//     std::cout << "Created an empty listpack." << std::endl;
//     lp.debug_print();

//     std::cout << "\nPushing 'hello' and 'world' to the back..." << std::endl;
//     lp.push_back("hello");
//     lp.push_back("world");
//     lp.debug_print();

//     std::cout << "\nPushing 'first' to the front..." << std::endl;
//     lp.push_front("first");
//     lp.debug_print();
    
//     std::cout << "\nInserting 'middle' before 'world'..." << std::endl;
//     char* world_ptr = lp.find_entry_from_head(2); // 'first', 'hello', 'world'
//     lp.insert(world_ptr, "middle");
//     lp.debug_print();

//     std::cout << "\nErasing 'hello' (element at index 1)..." << std::endl;
//     char* hello_ptr = lp.find_entry_from_head(1);
//     lp.erase(hello_ptr);
//     lp.debug_print();
    
//     std::cout << "\nVerifying content from head:" << std::endl;
//     for (int i = 0; i < lp.get_num_elements(); ++i) {
//         char* p = lp.find_entry_from_head(i);
//         std::cout << "Index " << i << ": " << lp.get_string(p).value_or("[error]") << std::endl;
//     }
    
//     std::cout << "\nVerifying content from tail:" << std::endl;
//     for (int i = 0; i < lp.get_num_elements(); ++i) {
//         char* p = lp.find_entry_from_tail(i);
//         std::cout << "Index from tail " << i << ": " << lp.get_string(p).value_or("[error]") << std::endl;
//     }

//     return 0;
// }











// // A faithful C++ replica of the Redis listpack data structure.
// // Manages a contiguous block of memory for storing a sequence of elements.
// class Listpack {
// private:
//     char* buffer_ = nullptr;
//     size_t capacity_ = 0; // The currently allocated size of the buffer

//     // Header constants
//     static const size_t TOTAL_BYTES_OFFSET = 0;
//     static const size_t NUM_ELEMENTS_OFFSET = 4;
//     static const size_t HEADER_SIZE = 6;
//     static const uint8_t EOF_BYTE = 0xFF;

//     // --- Private Helper Methods for Memory and Layout ---

//     // Directly set the 4-byte total size in the header
//     void set_total_bytes(uint32_t size) {
//         memcpy(buffer_ + TOTAL_BYTES_OFFSET, &size, sizeof(size));
//     }

//     // Directly set the 2-byte element count in the header
//     void set_num_elements(uint16_t count) {
//         memcpy(buffer_ + NUM_ELEMENTS_OFFSET, &count, sizeof(count));
//     }

//     // Ensure the buffer has enough allocated capacity for a required size.
//     // This is our manual replacement for std::vector's automatic growth.
//     bool ensure_capacity(size_t required_size) {
//         if (capacity_ >= required_size) return true;
//         size_t new_capacity = capacity_ > 0 ? capacity_ * 2 : 16;
//         if (new_capacity < required_size) new_capacity = required_size;
        
//         char* new_buffer = static_cast<char*>(realloc(buffer_, new_capacity));
//         if (!new_buffer) return false; // Allocation failure
        
//         buffer_ = new_buffer;
//         capacity_ = new_capacity;
//         return true;
//     }
    
//     // --- Private Encoding/Decoding Helpers ---
//     // (These are simplified for clarity but capture the essence)

//     // Calculate how many bytes are needed for the <encoding+data> part of an entry
//     size_t get_element_encoded_len(const std::string& s) const {
//         // Simplified: 1 byte for encoding type, then the string data
//         return 1 + s.length();
//     }
    
//     // Write the <encoding+data> part to the buffer
//     void encode_element(char* p, const std::string& s) {
//         // Simplified encoding: 0x00 for string
//         p[0] = 0x00; 
//         memcpy(p + 1, s.c_str(), s.length());
//     }
    
//     // Calculate how many bytes are needed for the <backlen> part
//     size_t get_backlen_size(size_t prev_entry_len) const {
//         // Simplified: always use 1 byte for backlen if length < 255
//         return (prev_entry_len < 255) ? 1 : 5;
//     }

//     // Write the <backlen> to the buffer
//     void encode_backlen(char* p, size_t prev_entry_len) {
//         // Simplified: just store the length directly
//         p[0] = static_cast<uint8_t>(prev_entry_len);
//     }
    
//     // Decode the <backlen> from the end of an entry
//     size_t decode_backlen(const char* p) const {
//         return static_cast<uint8_t>(p[0]);
//     }

// public:
//     // Constructor: Creates an empty Listpack
//     Listpack() {
//         size_t initial_size = HEADER_SIZE + sizeof(EOF_BYTE);
//         buffer_ = static_cast<char*>(malloc(initial_size));
//         if (!buffer_) throw std::bad_alloc();
//         capacity_ = initial_size;
//         set_total_bytes(initial_size);
//         set_num_elements(0);
//         buffer_[HEADER_SIZE] = EOF_BYTE;
//     }

//     // Destructor: Must free our manually managed memory
//     ~Listpack() {
//         free(buffer_);
//     }

//     // Rule of Five: Disable copy/move semantics to prevent issues with raw pointers
//     Listpack(const Listpack&) = delete;
//     Listpack& operator=(const Listpack&) = delete;
//     Listpack(Listpack&&) = delete;
//     Listpack& operator=(Listpack&&) = delete;

//     // --- Public API ---

//     uint16_t get_num_elements() const {
//         if (!buffer_) return 0;
//         uint16_t count;
//         memcpy(&count, buffer_ + NUM_ELEMENTS_OFFSET, sizeof(count));
//         return count;
//     }

//     uint32_t get_total_bytes() const {
//         if (!buffer_) return 0;
//         uint32_t size;
//         memcpy(&size, buffer_ + TOTAL_BYTES_OFFSET, sizeof(size));
//         return size;
//     }

//     // Insert a new element. This is the most complex public function.
//     // 'p' is the position to insert BEFORE.
//     bool insert(char* p, const std::string& value) {
//         uint16_t num_elements = get_num_elements();
        
//         // 1. Calculate required lengths
//         size_t element_len = get_element_encoded_len(value);
//         size_t backlen_len = get_backlen_size(element_len);
//         size_t new_entry_total_len = element_len + backlen_len;

//         size_t old_total_bytes = get_total_bytes();
//         size_t new_total_bytes = old_total_bytes + new_entry_total_len;
        
//         if (!ensure_capacity(new_total_bytes)) return false;

//         // 2. Make room by shifting memory
//         size_t insert_offset = p - buffer_;
//         memmove(buffer_ + insert_offset + new_entry_total_len, 
//                 buffer_ + insert_offset, 
//                 old_total_bytes - insert_offset);

//         // 3. Write the new entry into the created gap
//         char* write_ptr = buffer_ + insert_offset;
//         encode_element(write_ptr, value);
//         encode_backlen(write_ptr + element_len, element_len);
        
//         // NOTE: A full implementation would also update the backlen of the *next* element.
//         // This is a complex detail omitted for this step-by-step guide.

//         // 4. Update the header
//         set_total_bytes(new_total_bytes);
//         set_num_elements(num_elements + 1);
//         return true;
//     }
    
//     // Find the starting pointer of the Nth entry from the head
//     char* find_entry_from_head(int index) {
//         if (index < 0 || index >= get_num_elements()) return nullptr;
//         char* p = buffer_ + HEADER_SIZE;
//         for (int i = 0; i < index; ++i) {
//             // Skip to the next entry
//             size_t elen = 1 + strlen(p + 1); // Simplified decode
//             size_t blen = decode_backlen(p + elen);
//             p += elen + blen;
//         }
//         return p;
//     }
    
//     // Public wrappers for insertion
//     bool push_back(const std::string& value) {
//         char* eof_ptr = buffer_ + get_total_bytes() - 1;
//         return insert(eof_ptr, value);
//     }
    
//     // Decode and return the string at a given entry pointer
//     std::string get_string(const char* p) const {
//         // Simplified decode: assumes 1-byte encoding header
//         return std::string(p + 1, strlen(p + 1));
//     }
// };
