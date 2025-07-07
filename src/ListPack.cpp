#include "ListPack.h"
#include "CusDebug.h"

/*
| Encoding Type     | First Byte Pattern | Description           | Length Bytes | Total Header Size | Notes                                    |
| ----------------- | ------------------ | ----------------------| ------------ | ----------------- | ---------------------------------------- |
| **6-bit string**  | `10xxxxxx`         | 6 bits                | 0 extra      | 1 byte            | Supports strings of length **0–63**      |
| **12-bit string** | `1110xxxx`         | 8 bits                | 1 extra      | 2 bytes           | Supports strings of length **0–4095**    |
| **32-bit string** | `11110000`         | 32-bit                | 4 extra      | 5 bytes           | Supports very large strings (up to 4 GB) |
*/

/*
<----------------- Entry A (Total Length: 8 bytes) -----------------> <----------------- Entry B (Total Length: 12 bytes) ---------------->
+------------------------------------+--------------------------------+--------------------------------------+-------------------------------+
| <--- Content of A (7 bytes) --->   | <--- Backlen for A (1 byte) -->| <--- Content of B (11 bytes) -->   | <--- Backlen for B (1 byte) --> |
+------------------------------------+--------------------------------+--------------------------------------+-------------------------------+
                                    | Value stored here is 0,        |                                      | Value stored here is 8,       |
                                    | because nothing comes          |                                      | which is the total length     |
                                    | before Entry A.                |                                      | of Entry A.                   |

*/

ListPack::ListPack() {
    size_t initial_size = HEADER_SIZE + sizeof(EOF_B);
    buffer = static_cast<char*>(malloc(initial_size));
    if (!buffer) throw std::bad_alloc();
    capacity = initial_size;
    set_total_bytes(initial_size);
    set_num_elements(0);
    buffer[HEADER_SIZE] = EOF_B;
}

void ListPack::set_total_bytes(uint32_t size) {
    memcpy(buffer+TOTAL_BYTES_OFFSET,&size,sizeof(size));
}

void ListPack::set_num_elements(uint16_t count) {
    memcpy(buffer+NUM_ELEMENTS_OFFSET,&count,sizeof(count));
}

uint16_t ListPack::get_num_elements() {
    uint16_t count;
    memcpy(&count, buffer + NUM_ELEMENTS_OFFSET, sizeof(count));
    return count;
}

uint32_t ListPack::get_total_bytes() {
    uint32_t size;
    memcpy(&size, buffer + TOTAL_BYTES_OFFSET, sizeof(size));
    return size;
}

char* ListPack::insert(char* ptr, const std::string &str) {
    return replace_bytes(ptr,str,true);
}

char* ListPack::erase(char* p) { return erase_bytes(p); }

bool ListPack::check_capacity(size_t required) {
    if (capacity >= required) return true;
    size_t new_capacity = capacity > 0 ? capacity * 2 : 32;
    if (new_capacity < required) new_capacity = required;
    
    char* new_buffer = static_cast<char*>(realloc(buffer, new_capacity));
    if (!new_buffer) return false;
    
    buffer = new_buffer;
    capacity = new_capacity;
    return true;
}

void ListPack::buffer_encode_element(char* p, const std::string& s) {
    size_t len = s.length();
    unsigned char* up = reinterpret_cast<unsigned char*>(p);

    if (len < 64) {
        up[0] = BUFFER_ENCODING_6BIT_STR | len;
        memcpy(up + 1, s.c_str(), len);
    }
    else if (len < 4096) {
        up[0] = BUFFER_ENCODING_12BIT_STR | (len >> 8);
        up[1] = len & 0xFF;
        memcpy(up + 2, s.c_str(), len);
    }
    else {
        
        up[0] = BUFFER_ENCODING_32BIT_STR;
        uint32_t len32 = static_cast<uint32_t>(len);
        memcpy(up + 1, &len32, sizeof(len32));
        memcpy(up + 5, s.c_str(), len);
    }
}

void ListPack::buffer_encode_backlen(char* p, size_t len) {
    unsigned char* up = reinterpret_cast<unsigned char*>(p);
    while (true) {
        unsigned char byte = len & 0x7F;
        len >>= 7;
        if (len != 0) {
            byte |= 0x80;
        }
        *up = byte;
        up++;
        if (len == 0) break;
    }
}

size_t ListPack::get_encoded_content_len(const std::string& s) const {
    size_t len = s.length();

    if (len < 64) return 1 + len;
    else if (len < 4096) return 2 + len;
    return 1 + 4 + len;
}

size_t ListPack::buffer_read_len(const char** p) const {
    const unsigned char* up = reinterpret_cast<const unsigned char*>(*p);
    uint8_t first_byte = up[0];
    
    if ((first_byte & BUFFER_ENCODING_MASK) == BUFFER_ENCODING_12BIT_STR) {
        *p += 2;
        return ((first_byte & 0x0F) << 8) | up[1];
    }
    if (first_byte == BUFFER_ENCODING_32BIT_STR) {
        uint32_t len;
        memcpy(&len, up + 1, sizeof(uint32_t));
        *p += 5;
        return len;
    }
    return 0;
}

size_t ListPack::get_element_content_len(const char* p) const {
    const uint8_t enc = p[0];

    if ((enc & 0x80) == BUFFER_ENCODING_7BIT_UINT) return 1;
    if ((enc & BUFFER_ENCODING_MASK) == BUFFER_ENCODING_6BIT_STR) return 1 + (enc & 0x3F);
    if ((enc & BUFFER_ENCODING_MASK) == BUFFER_ENCODING_13BIT_INT) return 2;
    if ((enc & BUFFER_ENCODING_MASK) == BUFFER_ENCODING_12BIT_STR) {
        const char* temp_p = p;
        size_t len = buffer_read_len(&temp_p);
        return 2 + len;
    }
    
    if ((enc & BUFFER_ENCODING_INT_MASK) == 0xF0) {
        switch (enc) {
            case BUFFER_ENCODING_16BIT_INT: return 1 + 2;
            case BUFFER_ENCODING_24BIT_INT: return 1 + 3;
            case BUFFER_ENCODING_32BIT_INT: return 1 + 4;
            case BUFFER_ENCODING_64BIT_INT: return 1 + 8;
            case BUFFER_ENCODING_32BIT_STR: {
                const char* temp_p = p;
                size_t len = buffer_read_len(&temp_p);
                return 5 + len;
            }
        }
    }

    // Should be unreachable
    return 0;
}

// Returns how many bytes are used to encode the backlen at pointer p.
size_t ListPack::get_decode_backlen_size(const char* p) const {
    size_t len = 1;
    const unsigned char* up = reinterpret_cast<const unsigned char*>(p);
    while(*up & 0x80) {
        len++;
        up++;
    }
    return len;
}
 
size_t ListPack::get_encoded_backlen_size(size_t len) const {
    if (len < 128) return 1;
    if (len < 16384) return 2;
    if (len < 2097152) return 3;
    if (len < 268435456) return 4;
    return 5;
}

// Decode the backlen from the pointer p.
size_t ListPack::buffer_decode_backlen(const char* p) {
    size_t val = 0;
    size_t shift = 0;
    while (true) {
        val |= (size_t)(*p & 0x7F) << shift;
        if (!(*p & 0x80)) break;
        shift += 7;
        p++;
    }
    return val;
}

size_t ListPack::get_full_entry_len(const char* p) {
    size_t content_len = get_element_content_len(p);
    size_t backlen_len = get_decode_backlen_size(p + content_len);
    return content_len + backlen_len;
}

char* ListPack::replace_bytes(char* pos, const std::string& str, bool is_inserting = false) {
    size_t old_entry_len = 0; 
    if (!is_inserting && static_cast<uint8_t>(pos[0]) != EOF_B) old_entry_len = get_full_entry_len(pos);
    
    size_t prev_entry_backlen = 0;

    // size and layout of the NEW entry
    
    
    if (static_cast<uint8_t>(pos[0]) == EOF_B) {
        // push_back
        if (get_num_elements() > 0) {
            prev_entry_backlen =last_entry_size;
        }
    } else {
        // push front
        if (pos != (buffer + HEADER_SIZE)) {
            prev_entry_backlen = buffer_decode_backlen(reinterpret_cast<const char*>(pos + get_element_content_len(pos)));
        }
    }
    size_t new_element_content_len = get_encoded_content_len(str);
    size_t new_back_len = get_encoded_backlen_size(prev_entry_backlen);
    size_t new_entry_len = new_element_content_len + new_back_len;

    if(get_num_elements()==0) {
        last_entry_size= new_entry_len;
    }


    // memory shift needed
    size_t next_entry_backlen_delta = 0;
    char* next_p = pos + old_entry_len;

    // check the change in the backlen of the front entry;
    if (static_cast<uint8_t>(next_p[0]) != EOF_B) {
        // old backlen
        size_t old_next_backlen_size = get_encoded_backlen_size(old_entry_len);
        // new backlen
        size_t new_next_backlen_size = get_encoded_backlen_size(new_entry_len);
        next_entry_backlen_delta = new_next_backlen_size - old_next_backlen_size;
    }

    int64_t len_diff = new_entry_len - old_entry_len;
    size_t total_shift = len_diff + next_entry_backlen_delta;
    size_t old_total_bytes = get_total_bytes();
    size_t new_total_bytes = old_total_bytes + total_shift;
    
    ptrdiff_t pos_offset = pos - buffer;
    if (!check_capacity(new_total_bytes)) return nullptr;
    pos = buffer + pos_offset;
    
    next_p = pos + old_entry_len; // Recalculate next_p after realloc

    // Relocate memory shift
    if (total_shift != 0) {
        // The destination for the move is AFTER where our new entry will be.
        memmove(pos + new_entry_len, next_p, old_total_bytes - (next_p - buffer));
    }
    
    // Write the new element and backlen
    char* new_p = pos;
    buffer_encode_element(new_p, str);
    buffer_encode_backlen(new_p + new_element_content_len, prev_entry_backlen);

    // update the backlen of the element that comes AFTER our new one
    char* next_after_shift = new_p + new_entry_len;
    if (static_cast<uint8_t>(next_after_shift[0]) != EOF_B) {
        buffer_encode_backlen(next_after_shift + get_element_content_len(next_after_shift), new_entry_len);
    } else {
        last_entry_size=new_entry_len;
    }
    
    // update header
    set_total_bytes(new_total_bytes);
    if (is_inserting) {
        set_num_elements(get_num_elements() + 1);
    }
    
    return new_p;
}

char* ListPack::erase_bytes(char* pos) {
    if(static_cast<uint8_t>(pos[0])==EOF_B) return nullptr;

    size_t previos_entry_backlen = 0;
    if (pos != buffer + HEADER_SIZE) previos_entry_backlen = buffer_decode_backlen(pos + get_element_content_len(pos));
    
    size_t erase_len=get_full_entry_len(pos);

    char* forward_entry_pos=pos+erase_len;
    size_t forward_entry_backlen_delta=0;

    if(static_cast<uint8_t>(forward_entry_pos[0])!=EOF_B) {
        size_t old_forward_backlen=get_decode_backlen_size(forward_entry_pos+get_element_content_len(forward_entry_pos));
        size_t new_forward_backlen= get_encoded_backlen_size(previos_entry_backlen);

        forward_entry_backlen_delta=new_forward_backlen-old_forward_backlen;
    }
    else {
        if(get_num_elements() == 1) {
            last_entry_size = 0;
        } else {
            last_entry_size = previos_entry_backlen;
        }
    }
    
    // memory shift needed
    size_t total_shift=erase_len-forward_entry_backlen_delta;
    size_t old_total_bytes = get_total_bytes();
    ptrdiff_t next_offset = forward_entry_pos - buffer;

    if (static_cast<uint8_t>(forward_entry_pos[0]) != EOF_B) {
        buffer_encode_backlen(forward_entry_pos + get_element_content_len(forward_entry_pos), previos_entry_backlen);
    }

    // Relocate memory shift
    memmove(pos, buffer + next_offset + forward_entry_backlen_delta, old_total_bytes - (next_offset + forward_entry_backlen_delta));

    // Update header
    set_total_bytes(old_total_bytes - total_shift);
    set_num_elements(get_num_elements() - 1);
    return pos;
}

std::optional<std::string> ListPack::pop_front() {
    char* first_ptr = buffer + HEADER_SIZE;
    if(static_cast<uint8_t>(first_ptr[0])==EOF_B) {
        last_entry_size=0;
        DB("ListPack is empty, cannot pop front.");
        return std::nullopt;
    }
    std::optional<std::string> returnval=get_string(first_ptr);
    // std::cout << "Popped front value: " << returnval.value_or("[error]") << std::endl;
    if(get_num_elements() == 1) {
        last_entry_size = 0;
    }
    if(!erase_bytes(first_ptr)) {
        DB("Failed to erase the first entry in ListPack.");
        return std::nullopt;
    }
    return returnval;
}

std::optional<std::string> ListPack::pop_back() {
    if(get_num_elements() == 0) return std::nullopt;
    char* last_backlen=buffer+get_total_bytes()-last_entry_size-1;
    if(static_cast<uint8_t>(last_backlen[0])==EOF_B) return std::nullopt;
    std::optional<std::string> returnval=get_string(last_backlen);
    size_t last_backlen_size = buffer_decode_backlen(last_backlen + get_element_content_len(last_backlen));
    if(get_num_elements() == 1) {
        last_entry_size = 0;
    }
    if(!erase(last_backlen)) {
        return std::nullopt;
    }
    last_entry_size=last_backlen_size;
    return returnval;
}

bool ListPack::push_back(const std::string& value) {
    DB("Pushing to listpack size: " + std::to_string(get_num_elements()));
    char* eof_ptr = buffer + get_total_bytes() - 1;
    char* new_entry_ptr = insert(eof_ptr, value);
    if (!new_entry_ptr) {DB("NOT ABLE TO PUSH BACk")return false;}
    last_entry_size = get_full_entry_len(new_entry_ptr);
    char* man= find_entry_from_head(0); // Ensure the header is updated
    std::cout<< "First entry after push_back: " << get_string(man).value_or("[error]") << std::endl;
    DB("Last entry size after push_back: "+ std::to_string(last_entry_size));
    return true;
}

bool ListPack::push_front(const std::string& value) {
    char* first_ptr = buffer + HEADER_SIZE;
    bool is_empty=(get_num_elements() == 0);
    char* first_entry=insert(first_ptr, value);
    if(!first_entry) return false;
    if(is_empty) {
        last_entry_size = get_full_entry_len(first_entry);
    }
    return true;
}

char* ListPack::find_entry_from_head(int index) {
    if(index<0||index>=get_num_elements()) return nullptr;
    char* p=buffer+HEADER_SIZE;
    for(int x=0; x<index ; x++) {
        p+=get_full_entry_len(p);
    }
    return p;
}

std::optional<std::string> ListPack::get_string(const char* p) const {

    if (!p || static_cast<uint8_t>(p[0]) == EOF_B) {
        return std::nullopt;
    }

    const uint8_t enc = p[0];
    
    if ((enc & BUFFER_ENCODING_MASK) == BUFFER_ENCODING_6BIT_STR) {
        uint8_t len = enc & 0x3F;
        return std::string(p + 1, len);
    }

    if ((enc & BUFFER_ENCODING_INT_MASK) == 0xF0) {
        if (enc == BUFFER_ENCODING_32BIT_STR) {
            uint32_t len;
            memcpy(&len, p + 1, sizeof(uint32_t));
            return std::string(p + 5, len);
        }
    } else if ((enc & 0xF0) == BUFFER_ENCODING_12BIT_STR) {
        uint16_t len = ((enc & 0x0F) << 8) | static_cast<uint8_t>(p[1]);
        return std::string(p + 2, len);
    }
    
    return std::nullopt;
}

std::vector<std::string> ListPack::get_range(size_t st,size_t ed) {
    if (st > ed || ed >= get_num_elements()) {
        return {};
    }
    std::vector<std::string> returnval;
    char* st_entry=find_entry_from_head(st);
    while (st<=ed)
    {
        returnval.push_back(get_string(st_entry).value_or("[error]"));
        st_entry+=get_full_entry_len(st_entry);
        st++;
    }
    return returnval;
}