#pragma once

#include "WndDesign/frame/ScrollFrame.h"


using namespace WndDesign;


class BlockView;
class BlockListView;


class RootFrame : public ScrollFrame<Vertical> {
public:
	RootFrame();
	~RootFrame();

	// data
private:
	void Save();

	// child
private:
	ref_ptr<BlockListView> block_view;
private:
	BlockView& GetChild();

	// layout
private:
	Point ConvertToDescendentPoint(Point point, BlockView& block_view);
	Point ConvertToChildPoint(Point point) { return ConvertToDescendentPoint(point, GetChild()); }
private:
	virtual ref_ptr<WndObject> HitTest(Point& point) override { return this; }

	// focus
public:
	void CheckFocus(BlockView& block_view);

	// caret
private:
	ref_ptr<BlockView> caret_focus = nullptr;
public:
	ref_ptr<BlockView> GetCaretFocus() const { return caret_focus; }
	void SetCaretFocus(BlockView& block_view);
private:
	void SetCaret(Point point);
	void ClearCaret();

	// selection
private:
	ref_ptr<BlockView> selection_focus = nullptr;
public:
	ref_ptr<BlockView> GetSelectionFocus() const { return selection_focus; }
	void SetSelectionFocus(BlockView& block_view);
	void ClearSelectionFocus() { ClearSelection(); }
private:
	void BeginSelect();
	void DoSelect(Point point);
	void FinishSelect();
	void SelectMore();
	void ClearSelection();

	// drag and drop
private:
	ref_ptr<BlockView> drag_drop_focus = nullptr;
public:
	ref_ptr<BlockView> GetDragDropFocus() const { return drag_drop_focus; }
	void SetDragDropFocus(BlockView& block_view);
	void ClearDragDropFocus() { CancelDragDrop(); }
private:
	void DoDragDrop(Point point);
	void CancelDragDrop();
	void FinishDragDrop();

	// message
public:
	bool IsCtrlDown() const;
	bool IsShiftDown() const;
private:
	virtual void OnMouseMsg(MouseMsg msg) override;
	virtual void OnKeyMsg(KeyMsg msg) override;
	virtual void OnNotifyMsg(NotifyMsg msg) override;
};
