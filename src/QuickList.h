#pragma once

#include "ListPack.h"
#include <optional>


const size_t MAX_LISTPACK_ENTRIES = 256;
const size_t MAX_LISTPACK_BYTES = 8192; // 8 KB

class QuickList {
public:
    struct Node {
        ListPack lp;
        Node* prev = nullptr;
        Node* next = nullptr;
    };

    Node* head = nullptr;
    Node* tail = nullptr;
    size_t total_size = 0;


    void free_quicklist(){
        Node* current = head;
        while (current != nullptr) {
            Node* to_delete = current;
            current = current->next;
            delete to_delete;
        }
        head=current;
        tail=head;
    }

    size_t size() const { return total_size; }
    bool empty() const { return total_size == 0; }

    void push_back(const std::string& value);
    void push_front(const std::string& value);

    std::optional<std::string> pop_back();
    std::optional<std::string> pop_front();

    std::optional<std::string> at(size_t index);
    std::vector<std::string> range(size_t st,size_t ed);
};