#pragma once
#include "DataEntry.h"
#include <string>

extern size_t DEFAULT_TABLE_COUNT;
const size_t MAX_BUCKET_CHAIN_LENGTH = 12;

class Dict
{
public:
    DataNode ht[2];
    bool rehasing;
    int rehasingIndex;
    Dict( size_t size = DEFAULT_TABLE_COUNT,int deaf=0) {
        init_dict( size, deaf);
        rehasing=false;
        rehasingIndex=-1;
    }
    void init_dict( size_t size = DEFAULT_TABLE_COUNT,int deaf=0);
    void increaseHashTable();
    void rehash();
    void add_to_db_inter( const std::string& key, CusData value, int deaf );

    std::tuple<DataEntry*, int, size_t> get_from_db_inter( const std::string& key);
    std::vector<std::string>  get_all_from_db_inter();
    // ~Dict();
};