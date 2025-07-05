#include "Executer.h"
#include "CusDebug.h"

void add_to_db(Dict& dict, const std::string& key,const CusData& value, int deaf ) {
    if (dict.rehasing && deaf == 0)
        rehash(dict);
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
std::string get_queue_from_db(Dict& dict, const std::string& comm, const std::string& key) {
    auto headtup=get_from_db_inter(dict,key);
    DataEntry* entry=std::get<0>(headtup);
    if(entry==nullptr) {
        return "!!!NO QUEUE PRESENT!!!";
    } else {
        if(std::holds_alternative<CusQueue>(entry->value)) {
            CusQueue& queu=std::get<CusQueue>(entry->value);
            CusData returnval;
            if ( comm=="LPOP")
            {
                if(!cusqueue_pop_front(queu,returnval))
                {
                    return "!! NO ENTRY IN QUEUE !!";
                }
                return cusdata_to_string(returnval);
                
            } else if (comm=="RPOP") {
                if(!cusqueue_pop_front(queu,returnval))
                {
                    return "!! NO ENTRY IN QUEUE !!";
                }
                return cusdata_to_string(returnval);
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
        CusListNode* newNode = new CusListNode{value, nullptr, nullptr};
        CusQueue queu;
        queu.head=newNode;
        queu.tail=newNode;
        add_to_db(dict,key,queu);
        DB("New queu added to the dict")
    } else if (std::holds_alternative<CusQueue>(entry->value)) {
        CusQueue& queu = std::get<CusQueue>(entry->value);
        if (comm == "LPUSH") {
            // Insert at head
            cusqueue_push_front(queu,value);
            return "Value LPUSHed to queue";
        } else if (comm == "RPUSH") {
            // Insert at tail
            cusqueue_push_back(queu,value);
            return "Value RPUSHed to queue";
        }
    } else {
        return "Key exists but is not a queue";
    }
    
    return "";
}
