#pragma once
#include <variant>
#include <string>
#include "QuickList.h"
#include <memory>

class Dict;

enum CommandName {
    GET,        SET,
    LPUSH,      RPUSH,
    LPOP,       RPOP,
    EXISTS,
    PING,
    ECHO,
    DEL,
    TYPE,
    INCR,       DECR,
    LLEN,
    LRANGE,
    HSET,       HGET,       HDEL,       HGETALL,
};

struct PrasedCommand {
    CommandName name;
    std::vector<std::string> args;
};

inline CommandName str_to_CommandName(std::string&str) {
    if(str=="GET")                  return GET;
    else if (str=="SET")            return SET;
    else if (str=="LPUSH")          return LPUSH;
    else if (str=="RPUSH")          return RPUSH;
    else if (str=="LPOP")           return LPOP;
    else if (str=="RPOP")           return RPOP;
    else if (str=="EXISTS")         return EXISTS;
    else if (str=="PING")           return PING;
    else if (str=="ECHO")           return ECHO;
    else if (str=="DEL")            return DEL;
    else if (str=="TYPE")           return TYPE;
    else if (str=="INCR")           return INCR;
    else if (str=="DECR")           return DECR;
    else if (str=="LLEN")           return LLEN;
    else if (str=="LRANGE")         return LRANGE;
    else if (str=="HSET")           return HSET;
    else if (str=="HGET")           return HGET;
    else if (str=="HDEL")           return HDEL;
    else if (str=="HGETALL")        return HGETALL;
    else {
        throw std::runtime_error("Unknown command:" + str);
    }
}

using CusData = std::variant<int, std::string, QuickList, std::shared_ptr<Dict>>;