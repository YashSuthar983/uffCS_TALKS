#include "Executer.h"
#include "CusDebug.h"

void add_to_db(Dict& dict, const std::string& key,const CusData& value, int deaf ) {
    if (dict.rehasing && deaf == 0)
        rehash(dict);
    DB("adding to db")
    add_to_db_inter(dict,key,value,deaf);
}

std::string get_from_db(Dict& dict, const std::string& key) {
    DataEntry* head=std::get<0>(get_from_db_inter(dict,key));
    if(head==nullptr)
        return "Key not available in the database";
    return cusdata_to_string(head->value);
}

std::string del_from_db(Dict&dict,const std::string& key)
{
    std::tuple<DataEntry*, int, size_t> headpair = get_from_db_inter(dict, key);
    DataEntry* head = std::get<0>(headpair);
    int table_index = std::get<1>(headpair);
    size_t hash_index = std::get<2>(headpair);
    if (head == nullptr)
        return "---Key Not Available---";
    std::string returnval = head->key+" : "+cusdata_to_string(head->value);

    if (head->prev) {
        head->prev->next = head->next;
    } else {
        dict.ht[table_index].table[hash_index] = head->next;
    }
    if (head->next) {
        head->next->prev = head->prev;
    }

    delete head;
    dict.ht[table_index].element_count--;
    dict.ht[table_index].space_used--;
    return "DElETED {"+returnval+"}";
}

// QeueNode 
std::string pop_from_queue_db(Dict& dict, const std::string& comm, const std::string& key) {
    auto headtup=get_from_db_inter(dict,key);
    DataEntry* entry=std::get<0>(headtup);
    if(entry==nullptr) {
        return "!!!NO QUEUE PRESENT!!!";
    } else {
        if(std::holds_alternative<QuickList>(entry->value)) {
            QuickList& queu=std::get<QuickList>(entry->value);
            CusData returnval;
            if ( comm=="LPOP")
            {
                return queu.pop_front().value_or("[ERROR]");
                
            } else if (comm=="RPOP") {
                return  queu.pop_back().value_or("[ERROR]");
            }
        } else {
            return "Key exists but is not a queue";
        }
    }
    return "";
}

std::string add_queue_to_db(Dict& dict, const std::string& comm, const std::string& key, const CusData& value) {
    auto headtup = get_from_db_inter(dict, key);
    DataEntry* entry = std::get<0>(headtup);
    if (entry==nullptr)
    {
        QuickList newList;
        newList.push_back(cusdata_to_string(value));
        add_to_db(dict,key,newList);
        DB("New queu added to the dict")
    } else if (std::holds_alternative<QuickList>(entry->value)) {
        QuickList& queu = std::get<QuickList>(entry->value);
        if (comm == "LPUSH") {
            queu.push_front(cusdata_to_string(value));
            return "Value LPUSHed to queue";
        } else if (comm == "RPUSH") {
            queu.push_back(cusdata_to_string(value));
            return "Value RPUSHed to queue";
        }
    } else {
        return "Key exists but is not a queue";
    }
    return "";
}

std::string get_from_queue_db(Dict& dict,const std::string& key,size_t index) {
    auto headtup = get_from_db_inter(dict, key);
    DataEntry* entry = std::get<0>(headtup);
    if (std::holds_alternative<QuickList>(entry->value)) {
        QuickList& queu = std::get<QuickList>(entry->value);
        return queu.at(index).value_or("[ERROR]");
    } else {
        return "Key exists but is not a queue";
    }
}

std::vector<std::string> get_range_from_queu_db(Dict&dict,const std::string& key,size_t st,size_t ed) {
    auto headtup = get_from_db_inter(dict, key);
    DataEntry* entry = std::get<0>(headtup);
    if (std::holds_alternative<QuickList>(entry->value)) {
        QuickList& queu = std::get<QuickList>(entry->value);
        return queu.range(st,ed);
    } else {
        return {};
    }
}