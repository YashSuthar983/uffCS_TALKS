#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sstream>
#include "DataDict.h"
#include "CusDebug.h"
#include "Executer.h"

void test_db()
{

    Dict dictionary;
    
    DB("-------------Adding to queu--------")
    std::cout << add_queue_to_db(dictionary, "RPUSH", "hotel", "a") << std::endl;
    std::cout << add_queue_to_db(dictionary, "RPUSH", "hotel", "b") << std::endl;
    std::cout << add_queue_to_db(dictionary, "RPUSH", "hotel", "c") << std::endl;
    std::cout << add_queue_to_db(dictionary, "RPUSH", "hotel", "d") << std::endl;

    std::vector<std::string> range = get_range_from_queu_db(dictionary, "hotel", 0, 3);
    for(auto&x:range)
    {
        std::cout<<x<<", ";
    }
    std::cout<<std::endl;
    // // Simulating:
    // // LPOP hotel (4 times)
    std::cout << pop_from_queue_db(dictionary, "LPOP", "hotel") << std::endl; // d
    std::cout << pop_from_queue_db(dictionary, "LPOP", "hotel") << std::endl; // c
    std::cout << pop_from_queue_db(dictionary, "LPOP", "hotel") << std::endl; // b
    std::cout << pop_from_queue_db(dictionary, "LPOP", "hotel") << std::endl; // a

    // // Empty pop
    std::cout << pop_from_queue_db(dictionary, "LPOP", "hotel") << std::endl; // [ERROR]

    // // Rebuild queue
    std::cout << add_queue_to_db(dictionary, "RPUSH", "hotel", "a") << std::endl;
    std::cout << add_queue_to_db(dictionary, "LPUSH", "hotel", "b") << std::endl;
    std::cout << add_queue_to_db(dictionary, "LPUSH", "hotel", "b") << std::endl;
    std::cout << add_queue_to_db(dictionary, "LPUSH", "hotel", "bbb") << std::endl;
    // std::vector<std::string> range1 = get_range_from_queu_db(dictionary, "hotel", 0, 1);
    // for(auto&x:range1)
    // {
    //     std::cout<<x<<", ";
    // }
    // std::cout<<std::endl;
    // // Now:
    std::cout << pop_from_queue_db(dictionary, "LPOP", "hotel") << std::endl; // bbb
    std::cout << pop_from_queue_db(dictionary, "LPOP", "hotel") << std::endl; // a
    std::cout << pop_from_queue_db(dictionary, "LPOP", "hotel") << std::endl; // b
    std::cout << pop_from_queue_db(dictionary, "LPOP", "hotel") << std::endl; // [ERROR]
    std::cout << pop_from_queue_db(dictionary, "LPOP", "hotel") << std::endl; // [ERROR]
    
    DB("-------------Adding to hash--------")
    // Dict inner_dict;
    PrasedCommand cmd;
    cmd.name = HSET;
    cmd.args.push_back("user:1");
    cmd.args.push_back("a");
    cmd.args.push_back("aa");
    cmd.args.push_back("b");
    cmd.args.push_back("bb");
    cmd.args.push_back("c");
    cmd.args.push_back("cc");
    cmd.args.push_back("d");
    cmd.args.push_back("dd");
    std::cout << "Adding to hash: " << execute_cmd(cmd, dictionary) << std::endl;
    DB("-------------Getting from hash--------")
    cmd.name = HGET;
    cmd.args.clear();
    cmd.args.push_back("user:1");
    cmd.args.push_back("b");
    std::cout<< "Getting from hash: " << execute_cmd(cmd, dictionary) << std::endl;

    DB("-------------Del from hash--------")
    cmd.name = HDEL;
    cmd.args.clear();
    cmd.args.push_back("user:1");
    cmd.args.push_back("a");
    cmd.args.push_back("b");
    cmd.args.push_back("c");
    std::cout<< "Del from hash: " << execute_cmd(cmd, dictionary) <<std::endl;

    DB("-------------Getting all from hash--------")
    cmd.name = HGETALL;
    cmd.args.clear();
    cmd.args.push_back("user:1");
    std::cout<< "Getting all from hash: " << execute_cmd(cmd, dictionary) <<std::endl;
}