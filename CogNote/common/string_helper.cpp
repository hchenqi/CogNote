#include "string_helper.h"


std::vector<std::wstring> split_string_filtered(const std::wstring& str) {
	std::vector<std::wstring> result;
	std::wstring current_line;
	std::wstring::const_iterator begin = str.begin(), end = begin;
	while (end != str.end()) {
		if (iswcntrl(*end)) {
			current_line.append(begin, end);
			begin = end + 1;
			if (*end == L'\r' || *end == L'\n') {
				if (*end == L'\r' && begin != str.end() && *begin == L'\n') { ++begin; }
				result.push_back(std::move(current_line)); current_line.clear();
			}
			end = begin;
		} else {
			++end;
		}
	}
	current_line.append(begin, end);
	result.push_back(std::move(current_line));
	return result;
}
