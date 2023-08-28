#include "TextView.h"
#include "PairView.h"

#include "WndDesign/figure/shape.h"
#include "WndDesign/message/ime.h"
#include "WndDesign/system/clipboard.h"

#include "../common/string_helper.h"


BEGIN_NAMESPACE(Anonymous)

// style
struct Style : public TextView::Style {
	Style() {
		font.family(L"Times New Roman", L"SimSun").size(20);
	}
};

END_NAMESPACE(Anonymous)


TextView::TextView(Block& parent, std::wstring text) : Block(parent), EditBox(Style(), text) {}

PairView& TextView::GetParent() { return static_cast<PairView&>(Block::GetParent()); }

void TextView::Load() { Deserialize(block.read().first, text); EditBox::OnTextUpdate(); }

void TextView::Save() { block.write(Serialize(text), {}); }

void TextView::OnDraw(FigureQueue& figure_queue, Rect draw_region) {
	EditBox::OnDraw(figure_queue, draw_region);
	if (HasDragDropFocus()) {
		figure_queue.add(drag_drop_caret_region.point, new Rectangle(drag_drop_caret_region.size, drag_drop_caret_color));
	}
}

bool TextView::HitTestSelection(Point point) {
	for (auto& rect : selection_region_list) { if (rect.Contains(point)) { return true; } }
	return false;
}

void TextView::SelectMore() {
	if (HasCaretFocus()) { SelectWord(); SetSelectionFocus(); return; }
	if (HasSelectionFocus()) {
		if (selection_range == GetEntireRange()) { return SelectSelf(); }
		SelectAll();
	}
}

void TextView::RedrawDragDropCaretRegion() { Redraw(drag_drop_caret_region); }

void TextView::DoDragDrop(Block& source, Point point) {
	if (dynamic_cast<TextView*>(&source) == nullptr) { return ClearDragDropFocus(); }
	HitTestInfo info = text_block.HitTestPoint(point);
	if (HasSelectionFocus() && selection_range.left() <= info.range.begin && info.range.begin <= selection_range.right()) {
		ClearDragDropFocus();
	} else if (!HasDragDropFocus() || drag_drop_caret_position != info.range.begin) {
		SetDragDropFocus();
		RedrawDragDropCaretRegion();
		drag_drop_caret_position = info.range.begin;
		drag_drop_caret_region = Rect(info.region.point, Size(drag_drop_caret_width, info.region.size.height));
		RedrawDragDropCaretRegion();
	}
}

void TextView::CancelDragDrop() {
	RedrawDragDropCaretRegion(); drag_drop_caret_region = region_empty;
}

void TextView::FinishDragDrop(Block& source) {
	TextView& text_view = static_cast<TextView&>(source);
	if (&text_view == this) {
		if (drag_drop_caret_position < selection_range.left()) {
			std::rotate(text.begin() + drag_drop_caret_position, text.begin() + selection_range.left(), text.begin() + selection_range.right());
		} else {
			std::rotate(text.begin() + selection_range.left(), text.begin() + selection_range.right(), text.begin() + drag_drop_caret_position);
			drag_drop_caret_position -= selection_range.length;
		}
		OnTextUpdate();
	} else {
		TextBox::Insert(drag_drop_caret_position, text_view.TextBox::Substr(text_view.selection_range));
		text_view.TextBox::Erase(text_view.selection_range);
	}
	UpdateSelection(TextRange(drag_drop_caret_position, text_view.selection_range.length));
}

TextView& TextView::MergeBackWith(TextView& text_view) {
	TextBox::Insert(text.size(), text_view.text);
	return *this;
}

void TextView::Split() {
	std::wstring str;
	if (HasSelectionFocus()) {
		str = TextBox::Substr(selection_range.right());
		TextBox::Erase(selection_range.left());
	} else {
		str = TextBox::Substr(caret_position);
		TextBox::Erase(caret_position);
	}
	GetParent().InsertAfterSelfOrFront(str, IsCtrlDown()).SetCaret(0);
}

void TextView::Indent() {
	IsShiftDown() ? GetParent().IndentSelfShift() : GetParent().IndentSelf();
}

void TextView::Delete(bool is_backspace) {
	if (HasSelection()) {
		TextBox::Erase(selection_range);
		SetCaret(selection_range.begin);
	} else {
		if (is_backspace) {
			if (caret_position == 0) {
				size_t reverse_pos = text.length();
				TextView& text_view = GetParent().MergeBeforeSelf();
				text_view.SetCaret(text_view.text.length() - reverse_pos);
			} else {
				size_t caret_position_old = caret_position;
				SetCaret(caret_position - 1);
				TextBox::Erase(TextRange(caret_position, caret_position_old - caret_position));
			}
		} else {
			if (caret_position >= text.length()) {
				size_t pos = text.length();
				GetParent().MergeAfterSelf().SetCaret(pos);
			} else {
				TextBox::Erase(TextRange(caret_position, GetCharacterLength(caret_position)));
			}
		}
	}
}

void TextView::Paste() {
	std::wstring str; GetClipboardData(str);
	std::vector<std::wstring> text_list = split_string_filtered(str);
	if (text_list.size() == 1) {
		Insert(text_list.front());
	} else {
		if (HasSelectionFocus()) {
			str = TextBox::Substr(selection_range.right());
			TextBox::Replace(TextRange(selection_range.left(), -1), text_list.front());
		} else {
			str = TextBox::Substr(caret_position);
			TextBox::Replace(TextRange(caret_position, -1), text_list.front());
		}
		text_list.erase(text_list.begin()); text_list.back().insert(0, str);
		GetParent().InsertAfterSelf(text_list).SetCaret(str.size());
	}
}

void TextView::OnKeyMsg(KeyMsg msg) {
	switch (msg.type) {
	case KeyMsg::KeyDown:
		switch (msg.key) {
		case Key::Enter: Split(); break;
		case Key::Tab: Indent(); break;
		case Key::Backspace: Delete(true); break;
		case Key::Delete: Delete(false); break;

		case CharKey('X'): if (IsCtrlDown()) { Cut(); } break;
		case CharKey('C'): if (IsCtrlDown()) { Copy(); } break;
		case CharKey('V'): if (IsCtrlDown()) { Paste(); } break;
		}
		break;
	case KeyMsg::Char:
		if (IsCtrlDown()) { break; }
		if (!iswcntrl(msg.ch)) { Insert(msg.ch); };
		break;
	case KeyMsg::ImeBegin: OnImeBegin(); break;
	case KeyMsg::ImeString: OnImeString(); break;
	case KeyMsg::ImeEnd: OnImeEnd(); break;
	}
}
