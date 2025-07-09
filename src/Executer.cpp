#include "Executer.h"
#include "CusDebug.h"

void add_to_db(Dict& dict, const std::string& key, CusData value, int deaf ) {
    if (dict.rehasing && deaf == 0)
        dict.rehash();
    DB("adding to db")
    dict.add_to_db_inter(key, std::move(value), deaf);
}

std::string get_from_db(Dict& dict, const std::string& key) {
    DataEntry* head=std::get<0>(dict.get_from_db_inter(key));
    if(head==nullptr)
        return "Key not available in the database";
    return cusdata_to_string(head->value);
}

bool del_from_db(Dict&dict,const std::string& key)
{
    std::tuple<DataEntry*, int, size_t> headpair = dict.get_from_db_inter(key);
    DataEntry* head = std::get<0>(headpair);
    int table_index = std::get<1>(headpair);
    size_t hash_index = std::get<2>(headpair);
    if (head == nullptr)
        return false;
    std::string returnval = cusdata_to_string(head->value);

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
    return true;
}

// QeueNode 
std::string pop_from_queue_db(Dict& dict, const std::string& comm, const std::string& key) {
    auto headtup=dict.get_from_db_inter(key);
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

std::string add_queue_to_db(Dict& dict, const std::string& comm, const std::string& key, CusData value) {
    auto headtup = dict.get_from_db_inter(key);
    DataEntry* entry = std::get<0>(headtup);
    if (entry==nullptr)
    {
        QuickList newList;
        newList.push_back(cusdata_to_string(value));
        add_to_db(dict,key,std::move(newList));
        DB("New queu added to the dict")
    } else if (std::holds_alternative<QuickList>(entry->value)) {
        QuickList& queu = std::get<QuickList>(entry->value);
        if (comm == "LPUSH") {
            queu.push_front(cusdata_to_string(std::move(value)));
            return "Value LPUSHed to queue";
        } else if (comm == "RPUSH") {
            queu.push_back(cusdata_to_string(std::move(value)));
            return "Value RPUSHed to queue";
        }
    } else {
        return "Key exists but is not a queue";
    }
    return "";
}

std::string get_from_queue_db(Dict& dict,const std::string& key,size_t index) {
    auto headtup = dict.get_from_db_inter(key);
    DataEntry* entry = std::get<0>(headtup);
    if (std::holds_alternative<QuickList>(entry->value)) {
        QuickList& queu = std::get<QuickList>(entry->value);
        return queu.at(index).value_or("[ERROR]");
    } else {
        return "Key exists but is not a queue";
    }
}

std::vector<std::string> get_range_from_queu_db(Dict&dict,const std::string& key,size_t st,size_t ed) {
    auto headtup = dict.get_from_db_inter(key);
    DataEntry* entry = std::get<0>(headtup);
    if (std::holds_alternative<QuickList>(entry->value)) {
        QuickList& queu = std::get<QuickList>(entry->value);
        return queu.range(st,ed);
    } else {
        return {};
    }
}

std::string execute_hdict_cmd(Dict& in_dict,PrasedCommand& cmd) {

    switch (cmd.name)
    {
        case HSET: {
            for(int x=0;x<cmd.args.size();x+=2) {
                if(x+1>=cmd.args.size()) {
                    return "-ERR HSET requires key and value pairs\r\n";
                }
                auto entry_list = in_dict.get_from_db_inter(cmd.args[x]);
                if (std::get<0>(entry_list) == nullptr) {
                    add_to_db(in_dict, cmd.args[x], cmd.args[x+1]);
                } else {
                    DB("HSET: DUPLICATE key->" + cmd.args[x] + " value->" + cmd.args[x+1] + " to hash");
                    std::get<0>(entry_list)->value = cmd.args[x+1];
                }
                DB("HSET: Added key->" + cmd.args[x] + " value->" + cmd.args[x+1] + " to hash");
            }
            return "+OK\r\n";
        }
        case HGET: {
            if(cmd.args.size() != 1) {
                return "-ERR HGET requires key and field\r\n";
            }
            auto entry_list = in_dict.get_from_db_inter(cmd.args[0]);
            if (std::get<0>(entry_list) == nullptr) {
                return "$-1\r\n"; // Key not found
            } else {
                std::string result = cusdata_to_string(std::get<0>(entry_list)->value);
                return "$"+std::to_string(result.size())+"\r\n"+result+"\r\n"; 
            }
            break;
        }
        case HDEL: {
            if (cmd.args.size() < 1) {
                return "-ERR HDEL requires at least 1 argument\r\n";
            }
            int del_count = 0;
            for (const auto& field : cmd.args) {
                DB("HDEL: Trying to delete field->" + field + " from hash");
                auto entry_list = in_dict.get_from_db_inter(field);
                if (std::get<0>(entry_list) != nullptr) {
                    if (del_from_db(in_dict, field)) {
                        del_count++;
                    } else {
                        return "-ERR Key exists but is not a hash\r\n";
                    }
                }
            }
            
            return ":" + std::to_string(del_count) + "\r\n";
        }
        case HGETALL: {
            std::string response;
            int count = 0;
            std::vector<std::string> all_entries = in_dict.get_all_from_db_inter();
            for (size_t i = 0; i < all_entries.size(); i += 2) {
                const std::string& field = all_entries[i];
                const std::string& value = all_entries[i + 1];
                response += "$" + std::to_string(field.size()) + "\r\n" + field + "\r\n";
                response += "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
                count++;
            }
            return "*" + std::to_string(count * 2) + "\r\n" + response;

        }
        default:
            break;
    }
    return "-ERR unknown command\r\n";
}

std::string hdict_cmd(Dict& dict, PrasedCommand& cmd) {
    if (cmd.args.size() < 1) {
        return "-ERR HSET requires 2 arguments\r\n";
    }
    auto entry_list = dict.get_from_db_inter(cmd.args[0]);
    if (std::get<0>(entry_list) == nullptr) {
        if (cmd.name == HSET) {

            auto inner_dict = std::make_shared<Dict>();
            add_to_db(dict, cmd.args[0], inner_dict);

            // Consume inner dict name 
            PrasedCommand sub_cmd;
            sub_cmd.name = cmd.name;
            sub_cmd.args = std::vector<std::string>(cmd.args.begin() +1,cmd.args.end());
            return execute_hdict_cmd(*inner_dict, sub_cmd);
        } else {
            if (cmd.name == HGETALL)return "*0\r\n";
            if (cmd.name == HDEL) return":0\r\n";
            if (cmd.name == HGET) return "$-1\r\n";
        }
    } else {
        if (std::holds_alternative<std::shared_ptr<Dict>>(std::get<0>(entry_list)->value)) {
            Dict& entry_dict = *std::get<std::shared_ptr<Dict>>(std::get<0>(entry_list)->value);
            // Consume inner dict name 
            PrasedCommand sub_cmd;
            sub_cmd.name = cmd.name;
            sub_cmd.args = std::vector<std::string>(cmd.args.begin() +1,cmd.args.end());
            return execute_hdict_cmd(entry_dict,sub_cmd);
        } else {
            return "Key exists but is not a queue\r\n";
        }
    }
    // execute_hdict_cmd()
    return "-ERR\r\n";
}

std::string execute_cmd(PrasedCommand& cmd, Dict& dict) {
    switch (cmd.name) 
    {
        case LRANGE: {
            if (cmd.args.size() != 3) {
                return "-ERR LRANGE requires 3 arguments\r\n";
            }
            auto entry_list = dict.get_from_db_inter(cmd.args[0]);
            if (std::get<0>(entry_list) == nullptr) {
                return "$-1\r\n";
            } else {
                if (std::holds_alternative<QuickList>(std::get<0>(entry_list)->value)) {
                    QuickList node = std::get<QuickList>(std::get<0>(entry_list)->value);
                    size_t st = std::stoul(cmd.args[1]);
                    size_t ed = std::stoul(cmd.args[2]);
                    if(ed==-1) ed = node.total_size - 1;
                    auto range = node.range(st, ed);
                    std::string result;
                    for (const auto& val : range) {
                        result += "$" + std::to_string(val.size()) + "\r\n" + val + "\r\n";
                    }
                    return result;
                } else {
                    return "-ERR LRANGE requires a queue\r\n";
                }
            }
        }
        case LLEN: {
            if(cmd.args.size() !=1) {
                return "-ERR LLEN requires 1 argument\r\n";
            }
            auto entry_list= dict.get_from_db_inter(cmd.args[0]);
            if(std::get<0>(entry_list)==nullptr) {
                return "$-1\r\n";
            } else {
                if(std::holds_alternative<QuickList>(std::get<0>(entry_list)->value)) {
                    QuickList node = std::get<QuickList>(std::get<0>(entry_list)->value);
                    return ":"+std::to_string(node.total_size)+"\r\n";
                } else {
                    return "-ERR LLEN requires a queue\r\n";
                }
            }
        }
        case TYPE: {
            if (cmd.args.size() != 1) {
                return "-ERR TYPE requires 1 argument\r\n";
            }
            auto headtup = dict.get_from_db_inter(cmd.args[0]);
            DataEntry* entry = std::get<0>(headtup);
            if (entry == nullptr) {
                return ":none\r\n";
            }
            if(std::holds_alternative<QuickList>(entry->value)) {
                return ":queue\r\n";
            } else if(std::holds_alternative<int>(entry->value)) {
                return ":int\r\n";
            } else if(std::holds_alternative<std::string>(entry->value)) {
                return ":string\r\n";
            }
            return "-ERR unknown type\r\n";
        }
        case DEL: {
            if (cmd.args.size() <1) {
                return "-ERR DEL requires 1 argument\r\n";
            }
            int del_cnt=0;
            for(auto&x:cmd.args) {
                if(del_from_db(dict,x)) del_cnt++;
            }
            return  ":"+std::to_string(del_cnt)+ "\r\n";
        }
        case ECHO: {
            if (cmd.args.size() != 1) {
                return "-ERR ECHO requires 1 argument\r\n";
            }
            return "$"+std::to_string(cmd.args[0].size())+"\r\n"+cmd.args[0]+"\r\n";
        }
        case PING: {
            if(cmd.args.size() >0) {
                return "$"+std::to_string(cmd.args[0].size())+"\r\n"+cmd.args[0]+"\r\n";
            }
            return "+PONG\r\n";
        }
        case EXISTS: {
            if (cmd.args.size() != 1) {
                return "-ERR EXISTS requires 1 argument\r\n";
            }
            {
                auto headtup = dict.get_from_db_inter(cmd.args[0]);
                DataEntry* entry = std::get<0>(headtup);
                return entry ? ":1\r\n" : ":0\r\n";
            }
        }
        case GET: {
            if (cmd.args.size() != 1) {
                return "-ERR GET requires 1 argument\r\n";
            }
            std::string result = get_from_db(dict, cmd.args[0]);
            return "$"+std::to_string(result.size())+"\r\n"+result+"\r\n";
        }
        case SET: {
            if (cmd.args.size() != 2) {
                return "-ERR SET requires 2 arguments\r\n";
            }
            add_to_db(dict,cmd.args[0],std::move(cmd.args[1]));
            return "+OK\r\n";
        }
        case INCR:
        case DECR:{
            if(cmd.args.size() != 1) {
                return "-ERR INCR/DECR requires 1 argument\r\n";
            }
            auto headtup = dict.get_from_db_inter(cmd.args[0]);
            DataEntry* entry=std::get<0>(dict.get_from_db_inter(cmd.args[0]));
            if(entry==nullptr) {
                if(cmd.name==DECR) {
                    add_to_db(dict,cmd.args[0],-1);
                    return ":-1\r\n";
                } else if(cmd.name==INCR) {
                    add_to_db(dict,cmd.args[0],1);
                    return ":1\r\n";
                }
            } else {
                if(cmd.name==DECR) {
                    if(std::holds_alternative<int>(entry->value)) std::get<int>(entry->value)--;
                    else return "-ERR INCR/DECR requires INT\r\n";
                    return ":"+std::to_string(std::get<int>(entry->value))+"\r\n";
                } else if(cmd.name==INCR) {
                    if(std::holds_alternative<int>(entry->value)) std::get<int>(entry->value)++;
                    else return "-ERR INCR/DECR requires INT\r\n";
                    return ":"+std::to_string(std::get<int>(entry->value))+"\r\n";
                }
            }
            return "";
        }
        case LPUSH:
        case RPUSH: {
            if (cmd.args.size() < 2) {
                return "-ERR LPUSH/RPUSH requires key and at least one value\r\n";
            }
            // Optional: loop over cmd.args[1:] to push multiple values
            for(int x=1;x<cmd.args.size();x++) {
                add_queue_to_db(dict, cmd.name == LPUSH ? "LPUSH" : "RPUSH", cmd.args[0], std::move(cmd.args[x]));
            }
            return "+OK\r\n";
        }
        case LPOP:
        case RPOP: {
            if (cmd.args.size() != 1) {
                return "-ERR LPOP/RPOP requires 1 argument\r\n";
            }
            return pop_from_queue_db(dict, cmd.name == LPOP ? "LPOP" : "RPOP", cmd.args[0]);
        }
        case HSET:
        case HGET:
        case HDEL:
        case HGETALL: {
            return hdict_cmd(dict, cmd);
        }
        default:
            return "-ERR unknown command\r\n";
    }
}
