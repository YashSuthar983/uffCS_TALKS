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

inline std::string cusdata_to_string(const CusData& val)
{
    if (std::holds_alternative<int>(val))
        return std::to_string(std::get<int>(val));
    if (std::holds_alternative<std::string>(val))
        return std::get<std::string>(val);
    return {};
}