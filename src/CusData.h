#pragma once
#include <variant>
#include <string>
#include "QuickList.h"

using CusData = std::variant<int, std::string, QuickList>;