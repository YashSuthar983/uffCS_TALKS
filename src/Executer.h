#pragma once 
#include "DataDict.h"
#include "CusQueue.h"

void add_to_db(Dict& dict, const std::string& key,const CusData& value, int deaf=0);

std::string get_from_db(Dict& dict, const std::string& key);


std::string del_from_db(Dict&dict,const std::string& key);
std::string get_queue_from_db(Dict& dict, const std::string& comm, const std::string& key);
std::string add_queue_to_db(Dict& dict, const std::string& comm, const std::string& key,const CusData& value);
