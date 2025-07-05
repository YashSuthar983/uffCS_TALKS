/* #pragma once
#include <variant>
#include <string>
#include "CusData.h"

struct CusListNode;

struct CusQueue {
    CusListNode* head;
    CusListNode* tail;
};

struct CusListNode {
    CusData value;
    CusListNode* prev;
    CusListNode* next;
};


void cusqueue_push_back(CusQueue& queue, const CusData& value);
void cusqueue_push_front(CusQueue& queue, const CusData& value);
bool cusqueue_pop_front(CusQueue& queue, CusData& out_value);
bool cusqueue_pop_back(CusQueue& queue, CusData& out_value);
void cusqueue_free(CusQueue& queue);
 */