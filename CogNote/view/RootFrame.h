#pragma once

#include "message/mouse_helper.h"

#include "BlockStore/block_ref.h"

#include "WndDesign/frame/ScrollFrame.h"


using namespace BlockStore;


class ListView;
class BlockView;


class RootFrame : public ScrollFrame<Vertical> {
public:
	RootFrame(block_ref& ref);
	~RootFrame();

	// data
private:
	block_ref& ref;
public:
	void Save();

	// child
private:
	ref_ptr<ListView> child;
private:
	ListView& GetChild();
	BlockView& GetChildView();

	// layout
private:
	Point ConvertToDescendentPoint(Point point, WndObject& wnd);
	Point ConvertToChildPoint(Point point);

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
	bool drag_drop_begin = false;
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
private:
	bool is_ctrl_down = false;
	bool is_shift_down = false;
	MouseHelper mouse_helper;
public:
	bool IsCtrlDown() const { return is_ctrl_down; }
	bool IsShiftDown() const { return is_shift_down; }
private:
	virtual ref_ptr<WndObject> HitTest(MouseMsg& msg) override { return this; }
private:
	virtual void OnMouseMsg(MouseMsg msg) override;
	virtual void OnKeyMsg(KeyMsg msg) override;
	virtual void OnNotifyMsg(NotifyMsg msg) override;
};
