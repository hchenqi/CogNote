#include "BlockTextView.h"

#include "WndDesign/figure/shape.h"
#include "WndDesign/message/ime.h"
#include "WndDesign/system/clipboard.h"

#include "../common/string_helper.h"


BEGIN_NAMESPACE(WndDesign)

BEGIN_NAMESPACE(Anonymous)

// style
struct Style : public TextBlockStyle {
	Style() {
		paragraph.line_height(25px);
		font.family(L"Calibri", L"Segoe UI Emoji", L"DengXian").size(20);
	}
}style;

// caret
constexpr float caret_width = 1.0f;
constexpr Color caret_color = Color::DimGray;

size_t caret_position = 0;
Rect caret_region = region_empty;

// selection
constexpr Color selection_color = Color(Color::DimGray, 0x7f);

size_t selection_begin = 0;
size_t selection_range_begin = 0;
size_t selection_range_end = 0;

std::vector<TextBlock::HitTestInfo> selection_info;
Rect selection_region_union;

// drag and drop
constexpr float drag_drop_caret_width = 1.0f;
constexpr Color drag_drop_caret_color = Color::DimGray;

size_t drag_drop_caret_position = 0;
Rect drag_drop_caret_region = region_empty;

// ime
size_t ime_composition_begin = 0;
size_t ime_composition_end = 0;

END_NAMESPACE(Anonymous)


BlockTextView::BlockTextView(std::wstring text) : text(text), text_block(style, text) {
	word_break_iterator.SetText(text);
}

void BlockTextView::TextUpdated() {
	text_block.SetText(style, text);
	word_break_iterator.SetText(text);
	SizeUpdated(UpdateLayout());
	Redraw(region_infinite);
}

Size BlockTextView::UpdateLayout() {
	text_block.UpdateSizeRef(Size(width, length_max));
	if (HasCaretFocus()) { SetCaret(caret_position); }
	if (HasSelectionFocus()) { UpdateSelectionRegion(selection_range_begin, selection_range_end); }
	return Size(width, text_block.GetSize().height);
}

Size BlockTextView::OnSizeRefUpdate(Size size_ref) {
	width = size_ref.width;
	return UpdateLayout();
}

void BlockTextView::OnDraw(FigureQueue& figure_queue, Rect draw_region) {
	figure_queue.add(point_zero, new TextBlockFigure(text_block, style.font._color));
	if (HasCaretFocus()) {
		figure_queue.add(caret_region.point, new Rectangle(caret_region.size, caret_color));
	}
	if (HasSelectionFocus()) {
		for (auto& it : selection_info) {
			Rect& region = it.geometry_region; if (region.Intersect(draw_region).IsEmpty()) { continue; }
			figure_queue.add(region.point, new Rectangle(region.size, selection_color));
		}
	}
	if (HasDragDropFocus()) {
		figure_queue.add(drag_drop_caret_region.point, new Rectangle(drag_drop_caret_region.size, drag_drop_caret_color));
	}
}

void BlockTextView::RedrawCaretRegion() { Redraw(caret_region); }

void BlockTextView::SetCaret(const HitTestInfo& info) {
	SetCaretFocus();
	RedrawCaretRegion();
	caret_position = info.text_position;
	caret_region = Rect(info.geometry_region.point, Size(caret_width, info.geometry_region.size.height));
	RedrawCaretRegion();
}

void BlockTextView::SetCaret(size_t text_position) { SetCaret(text_block.HitTestTextPosition(text_position)); }

void BlockTextView::SetCaret(Point point) { SetCaret(text_block.HitTestPoint(point)); }

void BlockTextView::ClearCaret() { RedrawCaretRegion(); }

void BlockTextView::RedrawSelectionRegion() { Redraw(selection_region_union); }

void BlockTextView::UpdateSelectionRegion(size_t begin, size_t end) {
	SetSelectionFocus();
	RedrawSelectionRegion();
	selection_range_begin = begin; selection_range_end = end;
	selection_info.clear(); selection_region_union = region_empty;
	if (end == begin) { return; }
	selection_info = text_block.HitTestTextRange(begin, end - begin);
	for (auto& it : selection_info) {
		selection_region_union = selection_region_union.Union(it.geometry_region);
	}
	RedrawSelectionRegion();
}

void BlockTextView::SelectWord() {
	if (caret_position >= text.length()) { return; }
	TextRange word_range = word_break_iterator.Seek(caret_position);
	UpdateSelectionRegion(word_range.left(), word_range.right());
}

bool BlockTextView::HitTestSelection(Point point) {
	for (auto& it : selection_info) { if (it.geometry_region.Contains(point)) { return true; } }
	return false;
}

void BlockTextView::BeginSelect(BlockView& child) { selection_begin = caret_position; }

void BlockTextView::DoSelect(Point point) {
	size_t begin = selection_begin, end = text_block.HitTestPoint(point).text_position;
	if (end < begin) { std::swap(begin, end); }
	if (selection_range_begin == begin && selection_range_end == end) { return; }
	UpdateSelectionRegion(begin, end);
}

void BlockTextView::SelectMore() {
	if (HasSelectionFocus()) { SelectSelf(); }
	if (HasCaretFocus()) { SelectWord(); }
}

void BlockTextView::ClearSelection() {
	RedrawSelectionRegion(); selection_info.clear(); selection_region_union = region_empty;
	selection_begin = selection_range_begin = selection_range_end = 0;
}

void BlockTextView::RedrawDragDropCaretRegion() { Redraw(drag_drop_caret_region); }

void BlockTextView::DoDragDrop(BlockView& source, Point point) {
	HitTestInfo info = text_block.HitTestPoint(point);
	if (selection_range_begin <= info.text_position && info.text_position <= selection_range_end) {
		ClearDragDropFocus();
	} else if (!HasDragDropFocus() || drag_drop_caret_position != info.text_position) {
		SetDragDropFocus();
		RedrawDragDropCaretRegion();
		drag_drop_caret_position = info.text_position;
		drag_drop_caret_region = Rect(info.geometry_region.point, Size(drag_drop_caret_width, info.geometry_region.size.height));
		RedrawDragDropCaretRegion();
	}
}

void BlockTextView::CancelDragDrop() {
	RedrawDragDropCaretRegion(); drag_drop_caret_region = region_empty;
}

void BlockTextView::FinishDragDrop(BlockView& source) {
	BlockTextView& text_view = static_cast<BlockTextView&>(source);
	size_t selection_length = selection_range_end - selection_range_begin;
	if (&text_view == this) {
		if (drag_drop_caret_position < selection_range_begin) {
			std::rotate(text.begin() + drag_drop_caret_position, text.begin() + selection_range_begin, text.begin() + selection_range_end);
		} else {
			std::rotate(text.begin() + selection_range_begin, text.begin() + selection_range_end, text.begin() + drag_drop_caret_position);
			drag_drop_caret_position -= selection_length;
		}
		TextUpdated();
	} else {
		std::wstring str = text_view.text.substr(selection_range_begin, selection_length);
		InsertText(drag_drop_caret_position, str);
		text_view.DeleteText(selection_range_begin, selection_length);
	}
	UpdateSelectionRegion(drag_drop_caret_position, drag_drop_caret_position + selection_length);
	CancelDragDrop();
}

void BlockTextView::Insert(wchar ch) {
	if (HasSelectionFocus()) {
		ReplaceText(selection_range_begin, selection_range_end - selection_range_begin, ch);
		SetCaret(selection_range_begin + 1);
	} else {
		InsertText(caret_position, ch);
		SetCaret(caret_position + 1);
	}
}

void BlockTextView::Insert(std::wstring str) {
	if (HasSelectionFocus()) {
		ReplaceText(selection_range_begin, selection_range_end - selection_range_begin, str);
		SetCaret(selection_range_begin + str.length());
	} else {
		InsertText(caret_position, str);
		SetCaret(caret_position + str.length());
	}
}

void BlockTextView::Delete(bool is_backspace) {
	if (HasSelectionFocus()) {
		DeleteText(selection_range_begin, selection_range_end - selection_range_begin);
		SetCaret(selection_range_begin);
	} else {
		if (is_backspace) {
			if (caret_position == 0) { return; }
			size_t previous_caret_position = caret_position;
			SetCaret(caret_position - 1);
			DeleteText(caret_position, previous_caret_position - caret_position);
		} else {
			if (caret_position >= text.length()) { return; }
			DeleteText(caret_position, GetCharacterLength(caret_position));
		}
	}
}

void BlockTextView::OnEnter() {
	std::wstring str;
	if (HasSelectionFocus()) {
		str = text.substr(selection_range_end);
		DeleteText(selection_range_begin, -1);
	} else {
		str = text.substr(caret_position);
		DeleteText(caret_position, -1);
	}
}

void BlockTextView::OnImeBegin() {
	if (HasSelectionFocus()) {
		ime_composition_begin = selection_range_begin;
		ime_composition_end = selection_range_end;
	} else {
		ime_composition_begin = caret_position;
		ime_composition_end = ime_composition_begin;
	}
	ime.SetPosition(*this, caret_region.LeftBottom());
}

void BlockTextView::OnImeString() {
	std::wstring str = ime.GetString();
	ReplaceText(ime_composition_begin, ime_composition_end - ime_composition_begin, str);
	ime_composition_end = ime_composition_begin + str.length();
	SetCaret(ime_composition_begin + ime.GetCursorPosition());
}

void BlockTextView::OnImeEnd() {
	if (caret_position != ime_composition_end) { SetCaret(ime_composition_end); }
}

void BlockTextView::Cut() {
	if (HasSelectionFocus()) {
		Copy();
		Delete(false);
	}
}

void BlockTextView::Copy() {
	if (HasSelectionFocus()) {
		SetClipboardData(text.substr(selection_range_begin, selection_range_end - selection_range_begin));
	}
}

void BlockTextView::Paste() {

}

void BlockTextView::OnKeyMsg(KeyMsg msg) {
	switch (msg.type) {
	case KeyMsg::KeyDown:
		switch (msg.key) {
		case Key::Backspace: Delete(true); break;
		case Key::Delete: Delete(false); break;

		case CharKey('A'): if (IsCtrlDown()) { SelectSelf(); } break;
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


END_NAMESPACE(WndDesign)