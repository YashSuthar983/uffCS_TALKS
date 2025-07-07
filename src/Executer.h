#pragma once 
#include "DataDict.h"
#include <vector>
#include "CusData.h"

void add_to_db(Dict& dict, const std::string& key,const CusData& value, int deaf=0);

std::string get_from_db(Dict& dict, const std::string& key);


bool del_from_db(Dict&dict,const std::string& key);
std::string pop_from_queue_db(Dict& dict, const std::string& comm, const std::string& key);
std::string add_queue_to_db(Dict& dict, const std::string& comm, const std::string& key,const CusData& value);
std::string get_from_queue_db(Dict& dict,const std::string& key,size_t index);
std::vector<std::string> get_range_from_queu_db(Dict&dict,const std::string& key,size_t st,size_t ed);
std::string execute_cmd(PrasedCommand&cmd,Dict& dict);