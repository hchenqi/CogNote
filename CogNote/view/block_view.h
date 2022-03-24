#pragma once

#include "WndDesign/window/wnd_traits.h"
#include "BlockStore/block_ref.h"


using namespace WndDesign;
using BlockStore::block_ref;


class RootFrame;


class BlockView : public WndObject {
private:
	friend class RootFrame;

public:
	BlockView(RootFrame& root) : root(root), parent(nullptr) {}
	BlockView(BlockView& parent) : root(parent.root), parent(&parent) {}

	// context
private:
	RootFrame& root;
	ref_ptr<BlockView> parent;
protected:
	bool IsRoot() const { return parent == nullptr; }
	BlockView& GetParent() { if (IsRoot()) { throw std::invalid_argument("window is a root view"); } return *parent; }
protected:
	void SetChildParent(BlockView& child) { child.parent = this; }

	// data
protected:
	block_ref block;
protected:
	void DataModified() {}
	bool IsDataModified() {}
protected:
	static void LoadChild(BlockView& child, block_ref block) { child.block = block; child.Load(); }
	static block_ref GetChildRef(BlockView& child) { return child.block; }
private:
	virtual void Load() {}
	virtual void Save() {}

	// caret
protected:
	bool HasCaretFocus() const;
	void SetCaretFocus();
protected:
	static void SetChildCaret(BlockView& child, Point point) { child.SetCaret(point); }
private:
	virtual void SetCaret(Point point) {}
	virtual void ClearCaret() {}

	// selection
protected:
	bool HasSelectionFocus() const;
	void SetSelectionFocus();
	void ClearSelectionFocus();
private:
	void BeginSelect() { BeginSelect(*this); BeginSelectSelf(); }
	void BeginSelectSelf() { if (!IsRoot()) { parent->BeginSelect(*this); parent->BeginSelectSelf(); } }
protected:
	static void DoChildSelect(BlockView& child, Point point) { child.DoSelect(point); };
	void SelectSelf() { if (!IsRoot()) { parent->SelectChild(*this); } }
private:
	virtual bool HitTestSelection(Point point) { return false; }
	virtual void BeginSelect(BlockView& child) {}
	virtual void DoSelect(Point point) {}
	virtual void FinishSelect() {}
	virtual void SelectChild(BlockView& child) {}
	virtual void SelectMore() { SelectSelf(); }
	virtual void ClearSelection() {}

	// drag and drop
protected:
	bool HasDragDropFocus() const;
	void SetDragDropFocus();
	void ClearDragDropFocus();
protected:
	static void DoChildDragDrop(BlockView& child, BlockView& source, Point point) { child.DoDragDrop(source, point); };
private:
	virtual void DoDragDrop(BlockView& source, Point point) {}
	virtual void CancelDragDrop() {}
	virtual void FinishDragDrop(BlockView& source) {}

	// message
protected:
	bool IsCtrlDown() const;
	bool IsShiftDown() const;
};
