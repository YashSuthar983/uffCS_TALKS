// In quicklist.h, after the Listpack class definition
#pragma once

#include "ListPack.h"
#include <optional>

template <typename T> // We'll make it generic, but our example uses strings
class QuickList {
private:
    struct Node {
        Listpack lp;
        Node* prev = nullptr;
        Node* next = nullptr;
    };

    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    size_t total_size_ = 0;
    const int fill_factor_; // Max elements per Listpack node

public:
    // Constructor
    explicit QuickList(int fill_factor = 32) : fill_factor_(fill_factor) {
        if (fill_factor <= 0) {
            throw std::invalid_argument("Fill factor must be positive.");
        }
    }

    // Destructor to clean up all nodes
    ~QuickList() {
        Node* current = head_;
        while (current != nullptr) {
            Node* to_delete = current;
            current = current->next;
            delete to_delete;
        }
    }

    size_t size() const { return total_size_; }
    bool empty() const { return total_size_ == 0; }

    // --- Core API Methods ---

    // RPUSH equivalent
    void push_back(const std::string& value) {
        // Case 1: List is empty, create the first node
        if (tail_ == nullptr) {
            head_ = tail_ = new Node();
        } 
        // Case 2: Tail node is full, create a new node and link it
        else if (tail_->lp.get_num_elements() >= fill_factor_) {
            Node* new_node = new Node();
            tail_->next = new_node;
            new_node->prev = tail_;
            tail_ = new_node;
        }

        // In all cases, push the value into the tail node's listpack
        tail_->lp.push_back(value);
        total_size_++;
    }

    // LPOP equivalent (simplified)
    std::optional<std::string> pop_front() {
        if (head_ == nullptr) {
            return std::nullopt;
        }
        
        // For simplicity, we pop the whole first node if not empty.
        // A full impl would remove one element and only delete the node if it becomes empty.
        char* first_entry = head_->lp.find_entry_from_head(0);
        if (!first_entry) return std::nullopt; // Node is empty somehow
        
        std::string result = head_->lp.get_string(first_entry);
        
        // This is a simplification!
        // We just delete the whole first node for this example.
        Node* to_delete = head_;
        head_ = head_->next;
        if (head_) {
            head_->prev = nullptr;
        } else {
            tail_ = nullptr; // List is now empty
        }

        total_size_ -= to_delete->lp.get_num_elements();
        delete to_delete;

        return result;
    }
    
    // LINDEX equivalent
    std::optional<std::string> at(size_t index) const {
        if (index >= total_size_) {
            return std::nullopt;
        }

        Node* current_node = head_;
        size_t current_index = index;

        // Traverse nodes until we find the one containing our target index
        while (current_node && current_index >= current_node->lp.get_num_elements()) {
            current_index -= current_node->lp.get_num_elements();
            current_node = current_node->next;
        }

        if (!current_node) {
            return std::nullopt; // Should not happen with our size check
        }

        // Get the element from within the correct listpack
        char* entry_ptr = current_node->lp.find_entry_from_head(current_index);
        if (!entry_ptr) return std::nullopt;

        return current_node->lp.get_string(entry_ptr);
    }
};