#include "block.h"

#include <list>


BEGIN_NAMESPACE(Anonymous)

std::list<Block<>*> modified_set;

END_NAMESPACE(Anonymous)


void Block<>::DataModified() { save_error = false; if (modified == false) { modified = true; modified_set.emplace_front(this); } }
void Block<>::ResetModified() { save_error = false; if (modified == true) { modified = false; *std::find(modified_set.begin(), modified_set.end(), this) = nullptr; } }

void Block<>::DoSave() {
	try {
		Save();
		ResetModified();
	} catch (...) {
		save_error = true;
	}
}

void Block<>::SaveAll() {
	for (auto it = modified_set.begin(); it != modified_set.end();) {
		if (*it == nullptr) {
			modified_set.erase(it++);
		} else {
			(*(it++))->DoSave();
		}
	}
}
