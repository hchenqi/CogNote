#include "block_view.h"
#include "RootFrame.h"

#include <list>


BEGIN_NAMESPACE(Anonymous)

std::list<ref_ptr<BlockView>> modified_set;

END_NAMESPACE(Anonymous)


BlockView::~BlockView() { root.CheckFocus(*this); ResetModified(); }

void BlockView::DataModified() { save_error = false; if (modified == false) { modified = true; modified_set.emplace_front(this); } }
void BlockView::ResetModified() { save_error = false; if (modified == true) { modified = false; *std::find(modified_set.begin(), modified_set.end(), this) = nullptr; } }

void BlockView::DoSave() {
	try {
		Save();
		ResetModified();
	} catch (...) {
		save_error = true;
	}
	Redraw(region_infinite);
}

void BlockView::SaveAll() {
	for (auto it = modified_set.begin(); it != modified_set.end();) {
		if (*it == nullptr) {
			modified_set.erase(it++);
		} else {
			(*(it++))->DoSave();
		}
	}
}

bool BlockView::HasCaretFocus() const { return root.GetCaretFocus() == this; }
void BlockView::SetCaretFocus() { root.SetCaretFocus(*this); }
void BlockView::ClearSelectionFocus() { root.ClearSelectionFocus(); }
bool BlockView::HasSelectionFocus() const { return root.GetSelectionFocus() == this; }
void BlockView::SetSelectionFocus() { root.SetSelectionFocus(*this); }
bool BlockView::HasDragDropFocus() const { return root.GetDragDropFocus() == this; }
void BlockView::SetDragDropFocus() { root.SetDragDropFocus(*this); }
void BlockView::ClearDragDropFocus() { root.ClearDragDropFocus(); }

bool BlockView::IsCtrlDown() const { return root.IsCtrlDown(); }
bool BlockView::IsShiftDown() const { return root.IsShiftDown(); }
