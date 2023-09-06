#pragma once

#include <string>
#include <vector>


struct pair_data {
	std::wstring text;
	std::vector<pair_data> list;
};

using text_data = std::wstring;
using list_data = std::vector<pair_data>;

using local_data = list_data;

local_data convert_from_string(const std::wstring& str);
std::wstring convert_to_string(const local_data& data);
