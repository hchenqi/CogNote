#include "block.h"
#include "RootFrame.h"

#include <list>


BEGIN_NAMESPACE(Anonymous)

std::list<ref_ptr<Block>> modified_set;

END_NAMESPACE(Anonymous)


Block::~Block() { root.CheckFocus(*this); ResetModified(); }

void Block::DataModified() { save_error = false; if (modified == false) { modified = true; modified_set.emplace_front(this); } }
void Block::ResetModified() { save_error = false; if (modified == true) { modified = false; *std::find(modified_set.begin(), modified_set.end(), this) = nullptr; } }

void Block::DoSave() {
	try {
		Save();
		ResetModified();
	} catch (...) {
		save_error = true;
	}
}

void Block::SaveAll() {
	for (auto it = modified_set.begin(); it != modified_set.end();) {
		if (*it == nullptr) {
			modified_set.erase(it++);
		} else {
			(*(it++))->DoSave();
		}
	}
}

bool Block::HasCaretFocus() const { return root.GetCaretFocus() == this; }
void Block::SetCaretFocus() { root.SetCaretFocus(*this); }
void Block::ClearSelectionFocus() { root.ClearSelectionFocus(); }
bool Block::HasSelectionFocus() const { return root.GetSelectionFocus() == this; }
void Block::SetSelectionFocus() { root.SetSelectionFocus(*this); }
bool Block::HasDragDropFocus() const { return root.GetDragDropFocus() == this; }
void Block::SetDragDropFocus() { root.SetDragDropFocus(*this); }
void Block::ClearDragDropFocus() { root.ClearDragDropFocus(); }

bool Block::IsCtrlDown() const { return root.IsCtrlDown(); }
bool Block::IsShiftDown() const { return root.IsShiftDown(); }
