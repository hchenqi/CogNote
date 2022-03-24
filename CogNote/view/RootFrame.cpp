#include "RootFrame.h"
#include "BlockPairView.h"

#include "BlockStore/block_manager.h"

#include "WndDesign/message/ime.h"
#include "message/mouse_helper.h"


using BlockStore::block_manager;


BEGIN_NAMESPACE(Anonymous)

// drag and drop
bool drag_drop_begin = false;

// message
bool is_ctrl_down = false;
bool is_shift_down = false;
MouseHelper mouse_helper;

END_NAMESPACE(Anonymous)


RootFrame::RootFrame() : ScrollFrame(new BlockPairView(*this)) {
	cursor = Cursor::Text;
	ime.Enable(*this);
	block_manager.open("CogNote.db");
	BlockView::LoadChild(GetChild(), block_manager.get_root());
}

RootFrame::~RootFrame() { Save(); block_manager.close(); }

void RootFrame::Save() {
	BlockView::SaveAll();
	block_manager.set_root(BlockView::GetChildRef(GetChild()));
}

BlockView& RootFrame::GetChild() { return static_cast<BlockView&>(*child); }

Point RootFrame::ConvertToDescendentPoint(Point point, BlockView& block_view) {
	return point_zero + (point - ConvertDescendentPoint(block_view, point_zero));
}

void RootFrame::SetCaretFocus(BlockView& block_view) {
	if (caret_focus == &block_view) { return; }
	ClearCaret(); ClearSelection();
	caret_focus = &block_view;
}

void RootFrame::SetCaret(Point point) {
	GetChild().SetCaret(ConvertToChildPoint(point));
}

void RootFrame::ClearCaret() {
	if (caret_focus) { caret_focus->ClearCaret(); caret_focus = nullptr; }
}

void RootFrame::SetSelectionFocus(BlockView& block_view) {
	if (selection_focus == &block_view) { return; }
	ClearCaret(); ClearSelection();
	selection_focus = &block_view;
}

void RootFrame::BeginSelect() {
	if (caret_focus) { caret_focus->BeginSelect(); }
}

void RootFrame::DoSelect(Point point) {
	GetChild().DoSelect(ConvertToChildPoint(point));
}

void RootFrame::FinishSelect() {
	if (selection_focus) { selection_focus->FinishSelect(); }
}

void RootFrame::SelectMore() {
	if (caret_focus) {
		caret_focus->SelectMore();
	} else if (selection_focus) {
		selection_focus->SelectMore();
	}
}

void RootFrame::ClearSelection() {
	if (selection_focus) { selection_focus->ClearSelection(); selection_focus = nullptr; }
}

void RootFrame::SetDragDropFocus(BlockView& block_view) {
	if (drag_drop_focus == &block_view) { return; }
	CancelDragDrop();
	drag_drop_focus = &block_view;
}

void RootFrame::DoDragDrop(Point point) {
	GetChild().DoDragDrop(*selection_focus, ConvertToChildPoint(point));
}

void RootFrame::CancelDragDrop() {
	if (drag_drop_focus) { drag_drop_focus->CancelDragDrop(); drag_drop_focus = nullptr; }
}

void RootFrame::FinishDragDrop() {
	if (drag_drop_focus) { drag_drop_focus->FinishDragDrop(*selection_focus); CancelDragDrop(); }
}

bool RootFrame::IsCtrlDown() const { return is_ctrl_down; }

bool RootFrame::IsShiftDown() const { return is_shift_down; }

void RootFrame::OnMouseMsg(MouseMsg msg) {
	switch (msg.type) {
	case MouseMsg::LeftDown: SetCapture(); SetFocus(); break;
	case MouseMsg::LeftUp: ReleaseCapture(); break;
	case MouseMsg::WheelVertical: Scroll((float)-msg.wheel_delta); break;
	}
	switch (mouse_helper.Track(msg)) {
	case MouseHelperMsg::Down:
		if (selection_focus && selection_focus->HitTestSelection(ConvertToDescendentPoint(msg.point, *selection_focus))) {
			drag_drop_begin = true;
		} else {
			SetCaret(msg.point);
			BeginSelect();
		}
		break;
	case MouseHelperMsg::Click:
		if (drag_drop_begin) {
			drag_drop_begin = false;
			SetCaret(msg.point);
		}
		break;
	case MouseHelperMsg::MultipleClick:
		SelectMore();
		break;
	case MouseHelperMsg::Drag:
		if (drag_drop_begin) {
			DoDragDrop(msg.point);
		} else {
			DoSelect(msg.point);
		}
		break;
	case MouseHelperMsg::Drop:
		if (drag_drop_begin) {
			drag_drop_begin = false;
			FinishDragDrop();
		} else {
			FinishSelect();
		}
		break;
	}
}

void RootFrame::OnKeyMsg(KeyMsg msg) {
	switch (msg.type) {
	case KeyMsg::KeyDown:
		switch (msg.key) {
		case Key::Escape: if (drag_drop_begin) { drag_drop_begin = false; CancelDragDrop(); } break;

		case Key::Ctrl: is_ctrl_down = true; break;
		case Key::Shift: is_shift_down = true; break;

		case CharKey('A'): if (IsCtrlDown()) { SelectMore(); } break;
		case CharKey('S'): if (IsCtrlDown()) { Save(); } break;
		}
		break;
	case KeyMsg::KeyUp:
		switch (msg.key) {
		case Key::Ctrl: is_ctrl_down = false; break;
		case Key::Shift: is_shift_down = false; break;
		}
		break;
	}
	if (caret_focus) {
		caret_focus->OnKeyMsg(msg);
	} else if (selection_focus) {
		selection_focus->OnKeyMsg(msg);
	}
}

void RootFrame::OnNotifyMsg(NotifyMsg msg) {
	switch (msg) {
	case NotifyMsg::LoseFocus:
		is_ctrl_down = false; is_shift_down = false;
		ClearCaret(); ClearSelection();
		break;
	}
}
