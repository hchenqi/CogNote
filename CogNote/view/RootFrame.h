#pragma once

#include "WndDesign/frame/ScrollFrame.h"

#include "message/mouse_helper.h"


using namespace WndDesign;


class Block;
class ListView;


class RootFrame : public ScrollFrame<Vertical> {
public:
	RootFrame();
	~RootFrame();

	// data
private:
	void Save();

	// child
private:
	ref_ptr<ListView> child;
private:
	WndObject& GetChildWnd();
	Block& GetChildBlock();

	// layout
private:
	Point ConvertToDescendentPoint(Point point, WndObject& block_view);
	Point ConvertToChildPoint(Point point) { return ConvertToDescendentPoint(point, GetChildWnd()); }
private:
	virtual ref_ptr<WndObject> HitTest(Point& point) override { return this; }

	// focus
public:
	void CheckFocus(Block& block_view);

	// caret
private:
	ref_ptr<Block> caret_focus = nullptr;
public:
	ref_ptr<Block> GetCaretFocus() const { return caret_focus; }
	void SetCaretFocus(Block& block_view);
private:
	void SetCaret(Point point);
	void ClearCaret();

	// selection
private:
	ref_ptr<Block> selection_focus = nullptr;
public:
	ref_ptr<Block> GetSelectionFocus() const { return selection_focus; }
	void SetSelectionFocus(Block& block_view);
	void ClearSelectionFocus() { ClearSelection(); }
private:
	void BeginSelect();
	void DoSelect(Point point);
	void FinishSelect();
	void SelectMore();
	void ClearSelection();

	// drag and drop
private:
	bool drag_drop_begin = false;
	ref_ptr<Block> drag_drop_focus = nullptr;
public:
	ref_ptr<Block> GetDragDropFocus() const { return drag_drop_focus; }
	void SetDragDropFocus(Block& block_view);
	void ClearDragDropFocus() { CancelDragDrop(); }
private:
	void DoDragDrop(Point point);
	void CancelDragDrop();
	void FinishDragDrop();

	// message
private:
	bool is_ctrl_down = false;
	bool is_shift_down = false;
	MouseHelper mouse_helper;
public:
	bool IsCtrlDown() const { return is_ctrl_down; }
	bool IsShiftDown() const { return is_shift_down; }
private:
	virtual void OnMouseMsg(MouseMsg msg) override;
	virtual void OnKeyMsg(KeyMsg msg) override;
	virtual void OnNotifyMsg(NotifyMsg msg) override;
};
