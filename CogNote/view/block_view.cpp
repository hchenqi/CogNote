#include "block_view.h"

#include "RootFrame.h"


bool BlockView::HasCaretFocus() const { return root.GetCaretFocus() == this; }
void BlockView::SetCaretFocus() { root.SetCaretFocus(*this); }
void BlockView::ClearSelectionFocus() { root.ClearSelectionFocus(); }
bool BlockView::HasSelectionFocus() const { return root.GetSelectionFocus() == this; }
void BlockView::SetSelectionFocus() { root.SetSelectionFocus(*this); }
bool BlockView::HasDragDropFocus() const {	return root.GetDragDropFocus() == this; }
void BlockView::SetDragDropFocus() { root.SetDragDropFocus(*this); }
void BlockView::ClearDragDropFocus() { root.ClearDragDropFocus(); }

bool BlockView::IsCtrlDown() const { return root.IsCtrlDown(); }
bool BlockView::IsShiftDown() const { return root.IsShiftDown(); }
