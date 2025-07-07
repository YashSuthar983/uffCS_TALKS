#include "QuickList.h"
#include "CusDebug.h"

void QuickList::push_back(const std::string& value) {
    DB("PUSHING TO QUICKLIST")
    if (tail == nullptr) {
        head = tail = new Node();
    } 
    else if ((tail->lp.get_num_elements() >= MAX_LISTPACK_ENTRIES)
            || (tail->lp.get_total_bytes() >= MAX_LISTPACK_BYTES)) {
        
        DB("CREATING NEW LISTPACK")
        Node* new_node = new Node();
        tail->next = new_node;
        new_node->prev = tail;
        tail = new_node;
    }
    DB("PUshed back to the quicklist")
    tail->lp.push_back(value);
    total_size++;
}

void QuickList::push_front(const std::string& value) {
    if (head == nullptr) {
        head = tail = new Node();
    } 
    else if ((head->lp.get_num_elements() >= MAX_LISTPACK_ENTRIES)
            || (head->lp.get_total_bytes() >= MAX_LISTPACK_BYTES)) {

        DB("CREATING NEW LISTPACK")
        Node* new_node = new Node();
        new_node->next=head;
        new_node->prev=nullptr;
        head = new_node;
    }
    
    head->lp.push_front(value);
    total_size++;
}

std::optional<std::string> QuickList::pop_back() {
    if (tail == nullptr) {
        return std::nullopt;
    }
    std::optional<std::string> result = tail->lp.pop_back();
    total_size--;
    if(tail->lp.get_num_elements()==0)
    {
        Node* to_delete = tail;
        if (tail->prev==nullptr) {
            head = tail = nullptr;
        } else {
            tail = tail->prev;
            tail->next = nullptr;
        }
        delete to_delete;
    }
    return result;
}

std::optional<std::string> QuickList::pop_front() {
    if (head == nullptr) {
        return std::nullopt;
    }
    std::optional<std::string> result = head->lp.pop_front();
    total_size--;
    if(head->lp.get_num_elements()==0)
    {
        Node* to_delete = head;
        if (head->next==nullptr) {
            head = tail = nullptr;
        } else {
            head = head->prev;
            head->next = nullptr;
        }
        delete to_delete;
    }
    return result;
}
    
std::optional<std::string> QuickList::at(size_t index) {
    if (index >= total_size) {
        return std::nullopt;
    }

    Node* current_node = head; 

    while (current_node && index >= current_node->lp.get_num_elements()) {
        index -= current_node->lp.get_num_elements();
        current_node = current_node->next;
    }

    if (!current_node) {
        return std::nullopt;
    }

    char* entry_ptr = current_node->lp.find_entry_from_head(index);
    
    if (!entry_ptr) return std::nullopt;

    return current_node->lp.get_string(entry_ptr);
}

std::vector<std::string> QuickList::range(size_t st,size_t ed) {
    if (st > ed || ed > total_size) {
        return {"Give Proper range"};
    }

    size_t steps=ed-st+1;
    Node* current_node = head; 

    while (current_node && st >= current_node->lp.get_num_elements()) {
        st -= current_node->lp.get_num_elements();
        current_node = current_node->next;
    }

    if (!current_node) {
        return {};
    }

    std::vector<std::string> returnval;
    while (steps>0&&current_node)
    {
        size_t ed_node=st+steps-1;
        if(ed_node>current_node->lp.get_num_elements()-1) ed_node=current_node->lp.get_num_elements()-1;
        steps-=ed_node-st+1;
        std::vector<std::string> temp = current_node->lp.get_range(st, ed_node);
        returnval.insert(returnval.end(), temp.begin(), temp.end());
        st=0;
        current_node=current_node->next;
    }
    
    return returnval;
}
