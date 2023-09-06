#include "local_data.h"

#include <stack>


local_data convert_from_string(const std::wstring& str) {
	local_data result;

	std::stack<local_data*> context;
	context.push(&result);

	std::wstring current_line;
	size_t current_line_level = 0;

	for (const wchar_t* begin = str.data(), *curr = begin, *end = begin + str.length(); curr <= end;) {
		if (iswcntrl(*curr)) {
			current_line.append(begin, curr);
			begin = curr + 1;
			if (*curr == L'\0' || *curr == L'\r' || *curr == L'\n') {
				if (*curr == L'\r' && begin != end && *begin == L'\n') { ++begin; }

				while (current_line_level + 1 < context.size()) {
					context.pop();
				}
				local_data& top = *context.top();
				top.push_back({ current_line, {} });
				context.push(&top.back().list);

				current_line.clear();
				current_line_level = 0;
			} else if (*curr == L'\t' && current_line.empty()) {
				current_line_level++;
			}
			curr = begin;
		} else {
			++curr;
		}
	}

	return result;
}

std::wstring convert_to_string(const local_data& data) {
	std::wstring result;

	struct iter {
		local_data::const_iterator pos;
		local_data::const_iterator end;
	};

	std::stack<iter> context;
	context.push({ data.begin(), data.end() });

	while (!context.empty()) {
		iter current = context.top(); context.pop();
		if (current.pos == current.end) {
			continue;
		}

		const pair_data& item = *current.pos;
		result += std::wstring(context.size(), L'\t') + item.text + L'\n';

		context.push({ current.pos + 1, current.end });
		context.push({ item.list.begin(), item.list.end() });
	}

	return result;
}
