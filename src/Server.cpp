// TODO:
/*
| **Issue Type**  | **Area**    | **Problem/Comment**                               | **Fix/Improvement Suggestion**           |
| --------------- | ----------- | ------------------------------------------------- | ---------------------------------------- |
| Memory Leak     | Hash Table  | Not freeing all `DataEntry` objects on table free | Traverse and delete all entries          |
| Memory Leak     | Queues      | Not freeing all `CusListNode` on queue delete     | Traverse and delete all nodes            |
| Memory Overhead | DataEntry   | Extra pointers for chaining and index             | Acceptable for hash table, but review    |
| Performance     | Rehashing   | Incremental, but can cause latency spikes         | Consider background rehashing if needed  |
| Performance     | Queues      | Heap allocation for each node                     | Consider memory pooling for optimization |
| Memory Leak     | `free_dict` | Not implemented                                   | Implement full cleanup                   |

*/




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


#define PORT 2345
#define BUFFER_SIE 1024
#define MAX_EVE 10

size_t DEFAULT_TABLE_COUNT = 4;
bool debug=true;

enum ClientStatus {
    STATUS_OK,
    STATUS_NEEDS_CLOSE,
    STATUS_ERROR
};
// Command Handler

PrasedCommand parse_cmd(std::string str)
{
    std::vector<std::string> cmd;
    size_t pos = 0;
    bool is_include=false;
    while (true) {
        
        size_t end = str.find("\r\n", pos);
        if (end == std::string::npos) break;
        std::string token = str.substr(pos, end - pos);
        if(is_include) cmd.push_back(token);
        if(pos!=0)is_include=!is_include;
        pos = end+2;
    }
    PrasedCommand returnval;
    DB("printing cmd")
    for(auto& x:cmd) {
        if(x.empty()) continue;
        std::cout<<"Token : "<<x<<std::endl;
    }
    DB("printing cmd end")
    if (cmd.empty()) throw std::runtime_error("No command found in input string");

    try
    {
        returnval.name = str_to_CommandName(cmd[0]);
        for(size_t x=1;x<cmd.size();x++) returnval.args.push_back(cmd[x]);   
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
    }
    return returnval;
}

void setNonBlocking(int fd)
{
    int flags=fcntl(fd,F_GETFL,0);
    if(flags==-1) {
        std::cerr<<"fnctl F_GETFL"<<std::endl;
        exit(EXIT_FAILURE);
    }
    if(fcntl(fd,F_SETFL,flags|O_NONBLOCK)==-1) {
        std::cerr<<"fnctl D_STEFL and O_NONBLOCKING"<<std::endl;
        exit(EXIT_FAILURE);
    }
}

ClientStatus main_loop(int client_fd,Dict& dict)
{
    char buffer[BUFFER_SIE];
    ssize_t bytes_read;
    bytes_read=recv(client_fd,buffer,BUFFER_SIE-1,0);

    PrasedCommand parsed_cmd;
    std::string return_value;
    std::cout<<"Bytes read from fd "<<client_fd<<" : "<<bytes_read<<std::endl;
    
    if (bytes_read == 0|| bytes_read == -1) {
        std::cout << "Client " << client_fd << " closed connection." << std::endl;
        return STATUS_NEEDS_CLOSE;
    }

    buffer[bytes_read]='\0';
    std::cout<<"Received from fd "<<client_fd<<" : "<<buffer<<std::endl;
    try {
        parsed_cmd=parse_cmd(buffer);
        return_value=execute_cmd(parsed_cmd,dict);
    } catch(const std::runtime_error& e) {
        return_value = "ERROR: " + std::string(e.what());
    }
    

    if(bytes_read>0) {
        buffer[bytes_read]='\0';
        printf("Received from fd %d: %s\n", client_fd, buffer);

        // send data back c
        if(send(client_fd, return_value.c_str(), return_value.size(), 0) == -1)
        {
            std::cerr << "Failed to send response to client " << client_fd << std::endl;
            return STATUS_ERROR;
        } else {
            std::cout<<"Echoed to soklect "<<client_fd<<" "<<return_value<<std::endl;
        }
    }
    memset(buffer,0,BUFFER_SIE);
    return STATUS_OK;
}

void test_db()
{

    Dict dictionary;
    init_dict(dictionary);
    dictionary.rehasing=false;
    dictionary.rehasingIndex=-1;
    
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
    std::cout<<std::endl;
}

int main(int argc, char **argv) {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    // test_db();
    DB("Starting server...");
    int epoll_fd;

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cerr << "Failed to create server\n";
        return 1;
    }

    int reuse = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
        std::cerr << "Setsockopt failed\n";
        close(socket_fd);
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    setNonBlocking(socket_fd);
    if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind\n";
        close(socket_fd);
        return 1;
    }

    if (listen(socket_fd, SOMAXCONN) < 0) {
        std::cerr << "Listen failed\n";
        close(socket_fd);
        return 1;
    }

    if((epoll_fd=epoll_create1(EPOLL_CLOEXEC))==-1) {
        std::cerr<<"Epoll created failed"<<std::endl;
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    struct epoll_event event;
    struct epoll_event events[MAX_EVE];                                                                                                                                                                                                               
    

    event.events=EPOLLIN;
    event.data.fd=socket_fd;

    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,socket_fd,&event)==-1)
    {
        std::cerr<<"epoll_ctl: add listen_fd failed"<<std::endl;
        close(epoll_fd);
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    std::cout<<"Listening socket added to epoll"<<std::endl;

    Dict dictionary;
    init_dict(dictionary);
    dictionary.rehasing=false;
    dictionary.rehasingIndex=-1;

    while (true)
    {
        // return the no of ready events / connections
        int num_eve=epoll_wait(epoll_fd,events,MAX_EVE,-1);

        if(num_eve==-1)
        {
            std::cerr<<"Epoll wait failed"<<std::endl;
            close(epoll_fd);
            close(socket_fd);
            exit(EXIT_FAILURE);
        }

        for(int x=0;x<num_eve;x++)
        {
            int client_fd=events[x].data.fd;

            //EPOLler = error     |||    EPOLLHUP ==hang
            if((events[x].events&EPOLLERR)||(events[x].events&EPOLLHUP))
            {
                std::cerr<<"epoll error hangip :: "<<client_fd<<std::endl;
                epoll_ctl(epoll_fd,EPOLL_CTL_DEL,client_fd,NULL);
                close(client_fd);
                continue;
            }

            if(socket_fd==client_fd)
            {
                struct sockaddr_in clientAddr;
                
                socklen_t clientAdr_len=sizeof(clientAddr);
                while ((client_fd=accept(socket_fd,(struct sockaddr*)&clientAddr,&clientAdr_len))!=-1) {
                    std::cout<<"new Connection from "<<inet_ntoa(clientAddr.sin_addr)
                                <<":"<<ntohs(clientAddr.sin_port)<<" on fd "<<client_fd<<std::endl;


                    setNonBlocking(client_fd);

                    event.events=EPOLLIN;
                    event.data.fd=client_fd;
                    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_fd,&event)==-1) {
                        std::cerr<<"epoll ctl: add client_fd"<<std::endl;
                        close(client_fd);
                        continue;
                    }
                }

                if(client_fd==-1&&errno!=EAGAIN&& errno!=EWOULDBLOCK) {
                    std::cerr<<"accept failed"<<std::endl;
                }
                
            }
            else
            {

                // main block 
                ClientStatus status= main_loop(client_fd,dictionary);
                if(status==STATUS_NEEDS_CLOSE) {
                    std::cout<<"Closing client fd "<<client_fd<<std::endl;
                    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,client_fd,NULL);
                    close(client_fd);
                }
            }

        }

    }
    
    free(events);
    close(epoll_fd);
    close(socket_fd);
    std::cout<<"Shuting down server"<<std::endl;

    DB("Shutting server")
    return 0;   
}
