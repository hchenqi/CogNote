#include "block_view.h"
#include "RootFrame.h"

#include <unordered_set>


BEGIN_NAMESPACE(Anonymous)

std::unordered_set<ref_ptr<BlockView>> modified_set;

END_NAMESPACE(Anonymous)


void BlockView::ResetModified() { if (modified == true) { modified = false; modified_set.erase(this); } }
void BlockView::DataModified() { if (modified == false) { modified = true; modified_set.insert(this); } }
void BlockView::SaveAll() { while (!modified_set.empty()) { (*modified_set.begin())->DoSave(); } }

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
