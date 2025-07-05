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
#include "DataDict.h"
#include "CusDebug.h"
#include "Executer.h"

#define PORT 2345
#define BUFFER_SIE 1024
#define MAX_EVE 10



size_t DEFAULT_TABLE_COUNT = 4;

bool debug=true;

void setNonBlocking(int fd)
{
    int flags=fcntl(fd,F_GETFL,0);
    if(flags==-1)
    {
        std::cerr<<"fnctl F_GETFL"<<std::endl;
        exit(EXIT_FAILURE);
    }
    if(fcntl(fd,F_SETFL,flags|O_NONBLOCK)==-1)
    {
        std::cerr<<"fnctl D_STEFL and O_NONBLOCKING"<<std::endl;
        exit(EXIT_FAILURE);
    }
}

void main_loop(int client_fd)
{
    char buffer[BUFFER_SIE];
    ssize_t bytes_read;
    bytes_read=recv(client_fd,buffer,BUFFER_SIE-1,0);



    if(bytes_read>0)
    {
        buffer[bytes_read]='\0';
        printf("Received from fd %d: %s\n", client_fd, buffer);

        // send data back c
        if(send(client_fd,buffer,bytes_read, 0)==-1)
        {
            std::cerr<<"failed to send to "<<std::endl;
            close(client_fd);
        }
        else
        {
            std::cout<<"Echoed to soklect "<<client_fd<<" "<<buffer<<std::endl;
        }
    }
    else{
        close(client_fd);
    }
    memset(buffer,0,BUFFER_SIE);
}

// int main(int argc, char **argv) {
//     std::cout << std::unitbuf;
//     std::cerr << std::unitbuf;

//     // int epoll_fd;

//     // int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
//     // if (socket_fd < 0) {
//     //     std::cerr << "Failed to create server\n";
//     //     return 1;
//     // }

//     // int reuse = 1;
//     // if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
//     //     std::cerr << "Setsockopt failed\n";
//     //     close(socket_fd);
//     //     return 1;
//     // }

//     // struct sockaddr_in addr;
//     // addr.sin_family = AF_INET;
//     // addr.sin_port = htons(PORT);
//     // addr.sin_addr.s_addr = INADDR_ANY;

//     // setNonBlocking(socket_fd);
//     // if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
//     //     std::cerr << "Failed to bind\n";
//     //     close(socket_fd);
//     //     return 1;
//     // }

//     // if (listen(socket_fd, SOMAXCONN) < 0) {
//     //     std::cerr << "Listen failed\n";
//     //     close(socket_fd);
//     //     return 1;
//     // }

//     // if((epoll_fd=epoll_create1(EPOLL_CLOEXEC))==-1)
//     // {
//     //     std::cerr<<"Epoll created failed"<<std::endl;
//     //     close(socket_fd);
//     //     exit(EXIT_FAILURE);
//     // }

//     // struct epoll_event event;
//     // struct epoll_event events[MAX_EVE];                                                                                                                                                                                                               
    

//     // event.events=EPOLLIN;
//     // event.data.fd=socket_fd;

//     // if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,socket_fd,&event)==-1)
//     // {
//     //     std::cerr<<"epoll_ctl: add listen_fd failed"<<std::endl;
//     //     close(epoll_fd);
//     //     close(socket_fd);
//     //     exit(EXIT_FAILURE);
//     // }

//     // std::cout<<"Listening socket added to epoll"<<std::endl;


//     // while (true)
//     // {
//     //     // return the no of ready events / connections
//     //     int num_eve=epoll_wait(epoll_fd,events,MAX_EVE,-1);

//     //     if(num_eve==-1)
//     //     {
//     //         std::cerr<<"Epoll wait failed"<<std::endl;
//     //         close(epoll_fd);
//     //         close(socket_fd);
//     //         exit(EXIT_FAILURE);
//     //     }

//     //     for(int x=0;x<num_eve;x++)
//     //     {
//     //         int client_fd=events[x].data.fd;

//     //         //EPOLler = error     |||    EPOLLHUP ==hang
//     //         if((events[x].events&EPOLLERR)||(events[x].events&EPOLLHUP))
//     //         {
//     //             std::cerr<<"epoll error hangip :: "<<client_fd<<std::endl;
//     //             // epoll_ctl(epoll_fd,EPOLL_CTL_DEL,client_fd,NULL);
//     //             close(client_fd);
//     //             continue;
//     //         }

//     //         if(socket_fd==client_fd)
//     //         {
//     //             struct sockaddr_in clientAddr;
                
//     //             socklen_t clientAdr_len=sizeof(clientAddr);
//     //             while ((client_fd=accept(socket_fd,(struct sockaddr*)&clientAddr,&clientAdr_len))!=-1)
//     //             {
//     //                 std::cout<<"new Connection from "<<inet_ntoa(clientAddr.sin_addr)
//     //                             <<":"<<ntohs(clientAddr.sin_port)<<" on fd "<<client_fd<<std::endl;


//     //                 setNonBlocking(client_fd);

//     //                 event.events=EPOLLIN;
//     //                 event.data.fd=client_fd;
//     //                 if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_fd,&event)==-1)
//     //                 {
//     //                     std::cerr<<"epoll ctl: add client_fd"<<std::endl;
//     //                     close(client_fd);
//     //                     continue;
//     //                 }
//     //             }

//     //             if(client_fd==-1&&errno!=EAGAIN&& errno!=EWOULDBLOCK)
//     //             {
//     //                 std::cerr<<"accept failed"<<std::endl;
//     //             }
                
//     //         }
//     //         else
//     //         {

//     //             // main block 
//     //             main_loop(client_fd);
//     //         }

//     //     }

//     // }
    
//     // free(events);
//     // close(epoll_fd);
//     // close(socket_fd);
//     // std::cout<<"Shuting down server"<<std::endl;

//     Dict dictionary;
//     init_dict(dictionary);
//     dictionary.rehasing=false;
//     dictionary.rehasingIndex=-1;
//     add_to_db(dictionary, "name", "yash");
//     add_to_db(dictionary, "class", "12");
//     add_to_db(dictionary, "age", "18");
    
//     add_to_db(dictionary, "phone", "9649");
//     add_to_db(dictionary, "speaker", "jbl");
//     add_to_db(dictionary, "hello", "bye");
//     add_to_db(dictionary, "hello", "hope");
    
//     DB("-----------Printing database------------")
//     for(size_t x = 0; x < dictionary.ht[0].table_size; x++)
//     {
//         DataEntry* entry_to_print = dictionary.ht[0].table[x];
//         while (entry_to_print != nullptr) {
//             // Assuming key is std::string and value can be cast to char* for printing
//             DB(entry_to_print->key + " : " + cusdata_to_string(entry_to_print->value));
//             entry_to_print = entry_to_print->next;
//         }
//     }
//     DB("----------------------------------------")

//     DB("------------Now getting data------------")
//     std::cout<<get_from_db(dictionary,"name")<<std::endl;
//     std::cout<<get_from_db(dictionary,"class")<<std::endl;
//     std::cout<<get_from_db(dictionary,"sdf")<<std::endl;
//     std::cout<<get_from_db(dictionary,"phone")<<std::endl;
//     std::cout<<get_from_db(dictionary,"speaker")<<std::endl;
//     std::cout<<get_from_db(dictionary,"name")<<std::endl;
//     std::cout<<get_from_db(dictionary,"hello")<<std::endl;
//     std::cout<<get_from_db(dictionary,"age")<<std::endl;
//     DB("----------------------------------------")
    

//     DB("------------Now Deleting data------------")
//     std::cout<<del_from_db(dictionary,"name")<<std::endl;
//     std::cout<<del_from_db(dictionary,"class")<<std::endl;
//     std::cout<<del_from_db(dictionary,"sdf")<<std::endl;
//     std::cout<<del_from_db(dictionary,"phone")<<std::endl;
//     std::cout<<del_from_db(dictionary,"speaker")<<std::endl;
//     std::cout<<del_from_db(dictionary,"name")<<std::endl;
//     std::cout<<del_from_db(dictionary,"hello")<<std::endl;
//     std::cout<<del_from_db(dictionary,"age")<<std::endl;

//     // Checking acces
//     std::cout<<get_from_db(dictionary,"speaker")<<std::endl;
//     std::cout<<get_from_db(dictionary,"name")<<std::endl;
//     std::cout<<get_from_db(dictionary,"hello")<<std::endl;
//     std::cout<<get_from_db(dictionary,"age")<<std::endl;
//     DB("----------------------------------------")

//     std::cout<<add_queue_to_db(dictionary,"LPUSH","hotel","yash")<<std::endl;
//     std::cout<<add_queue_to_db(dictionary,"LPUSH","hotel","pra")<<std::endl;
//     std::cout<<add_queue_to_db(dictionary,"RPUSH","hotel","hush")<<std::endl;
//     std::cout<<get_queue_from_db(dictionary,"LPOP","hotel")<<std::endl;
//     std::cout<<get_queue_from_db(dictionary,"RPOP","hotel")<<std::endl;
//     std::cout<<get_queue_from_db(dictionary,"LPOP","hotel")<<std::endl;
//     std::cout<<get_queue_from_db(dictionary,"RPOP","hotel")<<std::endl;
//     DB("Shutting server")
//     return 0;   
// }










// In main.cpp

#include "QuickList.h"
#include "ListPack.h"

int main() {
    std::cout << ">>> Creating QuickList with a fill_factor of 3 <<<\n";
    QuickList<std::string> redis_list(3);

    std::cout << "Pushing 'a', 'b', 'c' (should fill one node)\n";
    redis_list.push_back("a");
    redis_list.push_back("b");
    redis_list.push_back("c");
    std::cout << "Current size: " << redis_list.size() << std::endl;

    std::cout << "\nPushing 'd' (should create a new node)\n";
    redis_list.push_back("d");
    std::cout << "Current size: " << redis_list.size() << std::endl;

    std::cout << "\nPushing 'e', 'f', 'g'\n";
    redis_list.push_back("e");
    redis_list.push_back("f");
    redis_list.push_back("g");
    std::cout << "Current size: " << redis_list.size() << std::endl;
    
    std::cout << "\n>>> Testing LINDEX (at) <<<\n";
    if(auto val = redis_list.at(0)) std::cout << "at(0): " << *val << std::endl;
    if(auto val = redis_list.at(3)) std::cout << "at(3): " << *val << std::endl;
    if(auto val = redis_list.at(6)) std::cout << "at(6): " << *val << std::endl;
    if(!redis_list.at(10)) std::cout << "at(10): Out of bounds (correct)\n";
    
    std::cout << "\n>>> Testing LPOP (simplified pop_front) <<<\n";
    if(auto val = redis_list.pop_front()) {
        std::cout << "Popped: " << *val << std::endl;
        std::cout << "New size: " << redis_list.size() << std::endl;
    }
    
    if(auto val = redis_list.at(0)) {
        std::cout << "New head at(0): " << *val << std::endl;
    }

    return 0;
}