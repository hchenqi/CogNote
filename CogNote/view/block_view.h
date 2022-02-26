#pragma once

#include "WndDesign/window/wnd_traits.h"


BEGIN_NAMESPACE(WndDesign)

class RootFrame;


class BlockView : public WndObject {
private:
	friend class RootFrame;

	// context
private:
	ref_ptr<RootFrame> root = nullptr;
	ref_ptr<BlockView> parent = nullptr;
private:
	bool IsRoot() const { return parent == nullptr; }

	// data
private:

	// caret
protected:
	bool HasCaretFocus() const;
	void SetCaretFocus();
protected:
	void SetChildCaret(BlockView& child, Point point) { child.SetCaret(point); }
private:
	virtual void SetCaret(Point point) {}
	virtual void ClearCaret() {}

	// selection
protected:
	bool HasSelectionFocus() const;
	void SetSelectionFocus();
protected:
	void BeginSelect() { if (!IsRoot()) { parent->BeginSelect(*this); } }
	void DoChildSelect(BlockView& child, Point point) { child.DoSelect(point); };
	void SelectSelf() { if (!IsRoot()) { parent->BeginSelect(*this); } }
private:
	virtual bool HitTestSelection(Point point) { return false; }
	virtual void BeginSelect(BlockView& child) {}
	virtual void DoSelect(Point point) {}
	virtual void FinishSelect() {}
	virtual void SelectChild(BlockView& child) {}
	virtual void SelectMore() { return SelectSelf(); }
	virtual void ClearSelection() {}

	// drag and drop
protected:
	bool HasDragDropFocus() const;
	void SetDragDropFocus();
	void ClearDragDropFocus();
protected:
	void DoChildDragDrop(BlockView& child, BlockView& source, Point point) { child.DoDragDrop(source, point); };
private:
	virtual void DoDragDrop(BlockView& source, Point point) {}
	virtual void CancelDragDrop() {}
	virtual void FinishDragDrop(BlockView& source) {}

	// message
protected:
	bool IsCtrlDown() const;
	bool IsShiftDown() const;
};


END_NAMESPACE(WndDesign)