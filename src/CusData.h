#pragma once
#include <variant>
#include <string>

struct  CusQueue;

using CusData = std::variant<int, std::string, CusQueue>;