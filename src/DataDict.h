#pragma once
#include "DataEntry.h"
#include <string>
#include <cstddef>

extern size_t DEFAULT_TABLE_COUNT;
const size_t MAX_BUCKET_CHAIN_LENGTH = 12;

struct DataNode {
    DataEntry** table;
    size_t table_size;
    size_t space_used;
    int element_count;
    size_t table_size_mask;
};

struct Dict {
    DataNode ht[2];
    bool rehasing;
    int rehasingIndex;
};


void init_dict(Dict& dict, size_t size = DEFAULT_TABLE_COUNT,int deaf=0);

void increaseHashTable();
void rehash(Dict& dict);

void add_to_db_inter(Dict& dict, const std::string& key,const CusData& value, int deaf );

std::tuple<DataEntry*, int, size_t> get_from_db_inter(Dict& dict, const std::string& key);

