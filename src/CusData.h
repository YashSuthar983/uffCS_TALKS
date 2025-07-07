#pragma once
#include <variant>
#include <string>
#include "QuickList.h"

enum CommandName {
    GET,
    SET,
    LPUSH,
    RPUSH,
    LPOP,
    RPOP,
    EXISTS,
    PING,
    ECHO,
    DEL,
    TYPE,
    INCR,
    DECR,
    LLEN,
    LRANGE
};

struct PrasedCommand {
    CommandName name;
    std::vector<std::string> args;
};

inline CommandName str_to_CommandName(std::string&str) {
    if(str=="GET") return GET;
    else if (str=="SET") return SET;
    else if (str=="LPUSH") return LPUSH;
    else if (str=="RPUSH") return RPUSH;
    else if (str=="LPOP") return LPOP;
    else if (str=="RPOP") return RPOP;
    else if (str=="EXISTS") return EXISTS;
    else if (str=="PING") return PING;
    else if (str=="ECHO") return ECHO;
    else if (str=="DEL") return DEL;
    else if (str=="TYPE") return TYPE;
    else if (str=="INCR") return INCR;
    else if (str=="DECR") return DECR;
    else if (str=="LLEN") return LLEN;
    else if (str=="LRANGE") return LRANGE;
    else {
        throw std::runtime_error("Unknown command:" + str);
    }
}

using CusData = std::variant<int, std::string, QuickList>;