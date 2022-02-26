#include "string_helper.h"


#pragma warning(push)
#pragma warning(disable : 26800)  // Use of a moved from object
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
				result.push_back(std::move(current_line));
			}
			end = begin;
		} else {
			++end;
		}
	}
	result.push_back(std::move(current_line.append(begin, end)));
	return result;
}
#pragma warning(pop)