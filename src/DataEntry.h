#pragma once
#include <string>
#include <variant>
#include "CusData.h"

struct DataEntry
{
    std::string key;
    CusData value;
    DataEntry* prev;
    DataEntry* next;
    int index;
};

struct DataNode {
    DataEntry** table;
    size_t table_size;
    int element_count;
    size_t table_size_mask;
};


inline std::string cusdata_to_string(const CusData& val)
{
    if (std::holds_alternative<int>(val))
        return std::to_string(std::get<int>(val));
    else if (std::holds_alternative<std::string>(val))
        return std::get<std::string>(val);
    else if (std::holds_alternative<QuickList>(val)) {
        return "[QUEU]";
    } else if (std::holds_alternative<std::shared_ptr<Dict>>(val)) {
        return "[INNER_DICT]";
    }
    return {};
}