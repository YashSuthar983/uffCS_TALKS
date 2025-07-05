/* #include "CusQueue.h"
#include "CusDebug.h"

void cusqueue_push_back(CusQueue& queue, const CusData& value) {
    CusListNode* newNode = new CusListNode{value, nullptr, nullptr};
    if (!queue.tail) {
        queue.head = queue.tail = newNode;
    } else {
        newNode->prev = queue.tail;
        queue.tail->next = newNode;
        queue.tail = newNode;
    }
}

void cusqueue_push_front(CusQueue& queue, const CusData& value) {
    CusListNode* newNode = new CusListNode{value, nullptr, nullptr};
    if (!queue.head) {
        queue.head = queue.tail = newNode;
    } else {
        newNode->next = queue.head;
        queue.head->prev = newNode;
        queue.head = newNode;
    }
}

bool cusqueue_pop_front(CusQueue& queue, CusData& out_value) {
    if (!queue.head) return false;
    CusListNode* node = queue.head;
    out_value = node->value;
    queue.head = node->next;
    if (queue.head) queue.head->prev = nullptr;
    else queue.tail = nullptr;
    delete node;
    return true;
}

bool cusqueue_pop_back(CusQueue& queue, CusData& out_value) {
    if (!queue.tail) return false;
    CusListNode* node = queue.tail;
    out_value = node->value;
    queue.tail = node->prev;
    if (queue.tail) queue.tail->next = nullptr;
    else queue.head = nullptr;
    delete node;
    return true;
}

void cusqueue_free(CusQueue& queue) {
    CusListNode* node = queue.head;
    while (node) {
        CusListNode* next = node->next;
        delete node;
        node = next;
    }
    queue.head = queue.tail = nullptr;
}

 */