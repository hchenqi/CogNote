#pragma once

#include "WndDesign/frame/ScrollFrame.h"


BEGIN_NAMESPACE(WndDesign)

class BlockView;


class RootFrame : public ScrollFrame<Vertical> {
public:
	RootFrame();

	// child
private:
	BlockView& GetChild();

	// layout
private:
	Point ConvertToChildPoint(Point point) { return point - GetChildOffset(); }
	Point ConvertToDescendentPoint(Point point, BlockView& block_view);
private:
	virtual ref_ptr<WndObject> HitTest(Point& point) override { return this; }

	// caret
private:
	ref_ptr<BlockView> caret_focus = nullptr;
public:
	ref_ptr<BlockView> GetCaretFocus() const { return caret_focus; }
	void SetCaretFocus(BlockView& block_view);
private:
	void ClearCaret();
	void SetCaret(Point point);

	// selection
private:
	ref_ptr<BlockView> selection_focus = nullptr;
public:
	ref_ptr<BlockView> GetSelectionFocus() const { return selection_focus; }
	void SetSelectionFocus(BlockView& block_view);
private:
	void ClearSelection();
	void BeginSelect();
	void DoSelect(Point point);
	void FinishSelect();
	void SelectMore();

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


END_NAMESPACE(WndDesign)