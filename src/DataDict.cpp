#include "DataDict.h"
#include "DataEntry.h"
#include <stdint.h>
#include <stddef.h>
#include "CusDebug.h"
#include <tuple>

uint32_t murmur32(const std::string& key) {
    size_t len=key.size();
    uint32_t seed=1374629187;
    const uint32_t C1 = 0xcc9e2d51;
    const uint32_t C2 = 0x1b873593;
    const uint32_t R1 = 15;
    const uint32_t R2 = 13;
    const uint32_t M  = 5;
    const uint32_t N  = 0xe6546b64;

    uint32_t hash = seed ^ (uint32_t)len;

    size_t i = 0;

    // First step: process in 4-byte chunks
    while (i + 4 <= len) {
        uint32_t k = (uint32_t)key[i] |
                     ((uint32_t)key[i + 1] << 8) |
                     ((uint32_t)key[i + 2] << 16) |
                     ((uint32_t)key[i + 3] << 24);

        k *= C1;
        k = (k << R1) | (k >> (32 - R1));
        k *= C2;

        hash ^= k;
        hash = (hash << R2) | (hash >> (32 - R2));
        hash = hash * M + N;

        i += 4;
    }

    // Second step: remaining 1-3 bytes
    if (i < len) {
        uint32_t k = 0;
        size_t j = 0;
        while (i < len) {
            k ^= (uint32_t)key[i] << (j * 8);
            i++;
            j++;
        }

        k *= C1;
        k = (k << R1) | (k >> (32 - R1));
        k *= C2;
        hash ^= k;
    }

    // Finalization
    hash ^= (uint32_t)len;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);

    return hash;
}

void init_dict(Dict& dict, size_t size,int deaf) {

    if (dict.ht[deaf].table != nullptr) {
        DB("WARNING: init_hash_table called on an already allocated table[" + std::to_string(deaf) + "]. Ensure proper cleanup.");
    }

    DB("Initializing table ht[" + std::to_string(deaf) + "] with size: " + std::to_string(size));
    dict.ht[deaf].table = (DataEntry**)calloc(size, sizeof(DataEntry*));
    if (!dict.ht[deaf].table) {
        DB("ERROR: Failed to allocate memory for hash table[" + std::to_string(deaf) + "].");
        exit(EXIT_FAILURE);
    }
    dict.ht[deaf].table_size = size;
    dict.ht[deaf].table_size_mask = size - 1;
    dict.ht[deaf].element_count = 0;
    dict.ht[deaf].space_used = 0;
}

void increaseHashTable()
{
    DEFAULT_TABLE_COUNT *= 2ll;
    DB("Increased Table Size = " + std::to_string(DEFAULT_TABLE_COUNT))
}

void rehash(Dict& dict)
{
    dict.rehasingIndex++;
    DataEntry* head = dict.ht[0].table[dict.rehasingIndex];
    while (head)
    {
        DataEntry* next_node = head->next;
        add_to_db_inter(dict, head->key, head->value, 1); // Move to ht[1]
        delete(head);
        head =next_node;
        
    }
    dict.ht[0].table[dict.rehasingIndex]=nullptr;
    if (dict.rehasingIndex + 1 == dict.ht[0].table_size)
    {
        dict.rehasing = false;
        dict.rehasingIndex = -1;
        free(dict.ht[0].table);
        dict.ht[0].table = nullptr;
        dict.ht[0] = dict.ht[1];
        dict.ht[1] = {nullptr, 0, 0, 0};
        DB("------------Rehasing Stopped-------------")
    }
    DB("Rehash-> Added table_index :" + std::to_string(dict.rehasingIndex));
}

void add_to_db_inter(Dict& dict, const std::string& key,const CusData& value, int deaf ) {
    // If rehashing, always add new keys to ht[1]

    int table_index = (dict.rehasing && deaf == 0) ? 1 : deaf;

    size_t hash_index = murmur32(key) & dict.ht[table_index].table_size_mask;
    DB("Got hash_index ="+std::to_string(hash_index)+"  in ht["+std::to_string(table_index)+"]")

    DataEntry* head = dict.ht[table_index].table[hash_index];

    // Check for the re assing
    if((head!=nullptr)&&(head->key==key))
    {
        DB("DEBUG : Reinit key->"+key+" value->"+cusdata_to_string(head->value)+"->"+cusdata_to_string(value));
        head->value=value;
        return;
    }

    if (!dict.rehasing&&head != nullptr && ((head->index >= MAX_BUCKET_CHAIN_LENGTH)||((((double)dict.ht[table_index].element_count)/dict.ht[table_index].table_size)>=1.245f))) {
        DB("---------Rehashing started----------")
        dict.rehasing = true;
        increaseHashTable();
        init_dict(dict, DEFAULT_TABLE_COUNT, 1);
        DB("( New dict size:"+std::to_string(DEFAULT_TABLE_COUNT)+")")
        dict.rehasingIndex=-1;
        rehash(dict);
        add_to_db_inter(dict,key,value,1);
        return;
    }

    DataEntry* entry = new DataEntry();
    entry->key = key;
    entry->value = value;
    entry->next = head;
    if (head == nullptr) {
        entry->index = 1;
        dict.ht[table_index].space_used++;
    }
    else {
        entry->index = head->index + 1;
        head->prev=entry;
    }

    dict.ht[table_index].table[hash_index] = entry;
    dict.ht[table_index].element_count++;
    DB("DATA ADDED:: base (key->" + key + " value->" + cusdata_to_string(value) + ")");
}

std::tuple<DataEntry*, int, size_t> get_from_db_inter(Dict& dict, const std::string& key)
{
    size_t hash_index;
    DataEntry* head;
    if (dict.rehasing) {
        hash_index = murmur32(key) & dict.ht[1].table_size_mask;
        head = dict.ht[1].table[hash_index];
        while (head)
        {
            if (head->key == key) return std::make_tuple(head, 1, hash_index);
            head = head->next;
        }
    }
    hash_index = murmur32(key) & dict.ht[0].table_size_mask;
    head = dict.ht[0].table[hash_index];
    while (head)
    {
        if (head->key == key) return std::make_tuple(head, 0, hash_index);
        head = head->next;
    }
    return std::make_tuple(nullptr, -1, 0);
}
