#include "TextView.h"
#include "PairView.h"

#include "BlockStore/layout_traits_stl.h"

#include "WndDesign/figure/shape.h"
#include "WndDesign/message/ime.h"
#include "WndDesign/system/clipboard.h"

#include "../common/string_helper.h"


BEGIN_NAMESPACE(Anonymous)

// style
struct Style : public TextBlockStyle {
	Style() {
		paragraph.line_height(25px);
		font.family(L"Times New Roman", L"SimSun").size(20);
	}
}style;

constexpr Color border_color_unsaved = Color(Color::Green, 63);
constexpr Color border_color_error = Color(Color::Red, 63);

// caret
constexpr float caret_width = 1.0f;
constexpr Color caret_color = Color::DimGray;

size_t caret_position = 0;
Rect caret_region = region_empty;

// selection
constexpr Color selection_color = Color(Color::DimGray, 0x7f);

size_t selection_begin = 0;
size_t selection_range_begin = 0;
size_t selection_range_length = 0;

std::vector<TextBlock::HitTestInfo> selection_info;
Rect selection_region_union;

inline size_t get_selection_range_end() { return selection_range_begin + selection_range_length; }
inline bool PositionCoveredBySelection(size_t pos) { return selection_range_begin <= pos && pos <= get_selection_range_end(); }

// drag and drop
constexpr float drag_drop_caret_width = 1.0f;
constexpr Color drag_drop_caret_color = Color::DimGray;

size_t drag_drop_caret_position = 0;
Rect drag_drop_caret_region = region_empty;

// ime
size_t ime_composition_begin = 0;
size_t ime_composition_length = 0;

inline size_t get_ime_composition_end() { return ime_composition_begin + ime_composition_length; }

END_NAMESPACE(Anonymous)


TextView::TextView(BlockView& parent, std::wstring text) :
	BlockView(parent), text(text), text_block(style, text) {
	word_break_iterator.SetText(text);
}

PairView& TextView::GetParent() { return static_cast<PairView&>(BlockView::GetParent()); }

void TextView::Load() { block.read(text); TextUpdated(); }

void TextView::Save() { block.write(text); }

void TextView::TextUpdated() {
	text_block.SetText(style, text);
	word_break_iterator.SetText(text);
	SizeUpdated(UpdateLayout());
	Redraw(region_infinite);
}

Size TextView::UpdateLayout() {
	text_block.UpdateSizeRef(Size(size.width, length_max));
	if (HasCaretFocus()) { SetCaret(caret_position); }
	if (HasSelectionFocus()) { UpdateSelectionRegion(selection_range_begin, selection_range_length); }
	return size.height = text_block.GetSize().height, size;
}

Size TextView::OnSizeRefUpdate(Size size_ref) {
	size.width = size_ref.width;
	return UpdateLayout();
}

void TextView::OnDraw(FigureQueue& figure_queue, Rect draw_region) {
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
	if (IsModified()) {
		figure_queue.add(point_zero, new Rectangle(size, 1.0f, HasSaveError() ? border_color_error : border_color_unsaved));
	}
}

void TextView::RedrawCaretRegion() { Redraw(caret_region); }

void TextView::SetCaret(const HitTestInfo& info) {
	SetCaretFocus();
	RedrawCaretRegion();
	caret_position = info.text_position;
	caret_region = Rect(info.geometry_region.point, Size(caret_width, info.geometry_region.size.height));
	RedrawCaretRegion();
}

void TextView::SetCaret(size_t text_position) { SetCaret(text_block.HitTestTextPosition(text_position)); }

void TextView::SetCaret(Point point) { SetCaret(text_block.HitTestPoint(point)); }

void TextView::ClearCaret() { RedrawCaretRegion(); }

void TextView::RedrawSelectionRegion() { Redraw(selection_region_union); }

void TextView::UpdateSelectionRegion(size_t begin, size_t length) {
	SetSelectionFocus();
	RedrawSelectionRegion();
	selection_range_begin = begin; selection_range_length = length;
	selection_info.clear(); selection_region_union = region_empty;
	if (length == 0) { return; }
	selection_info = text_block.HitTestTextRange(begin, length);
	for (auto& it : selection_info) {
		selection_region_union = selection_region_union.Union(it.geometry_region);
	}
	RedrawSelectionRegion();
}

void TextView::SelectWord() {
	if (text.empty()) { return SelectSelf(); }
	TextRange word_range = word_break_iterator.Seek(caret_position >= text.length() ? caret_position - 1 : caret_position);
	UpdateSelectionRegion(word_range.begin, word_range.length);
}

bool TextView::HitTestSelection(Point point) {
	for (auto& it : selection_info) { if (it.geometry_region.Contains(point)) { return true; } }
	return false;
}

void TextView::BeginSelect(BlockView& child) { selection_begin = caret_position; }

void TextView::DoSelect(Point point) {
	size_t begin = selection_begin, end = text_block.HitTestPoint(point).text_position;
	if (end < begin) { std::swap(begin, end); }
	UpdateSelectionRegion(begin, end - begin);
}

void TextView::SelectMore() {
	if (HasCaretFocus()) { return SelectWord(); }
	if (HasSelectionFocus()) {
		if (selection_range_begin == 0 && selection_range_length == text.size()) { return SelectSelf(); }
		UpdateSelectionRegion(0, text.size());
	}
}

void TextView::ClearSelection() { RedrawSelectionRegion(); }

void TextView::RedrawDragDropCaretRegion() { Redraw(drag_drop_caret_region); }

void TextView::DoDragDrop(BlockView& source, Point point) {
	if (dynamic_cast<TextView*>(&source) == nullptr) { return ClearDragDropFocus(); }
	HitTestInfo info = text_block.HitTestPoint(point);
	if (HasSelectionFocus() && PositionCoveredBySelection(info.text_position)) {
		ClearDragDropFocus();
	} else if (!HasDragDropFocus() || drag_drop_caret_position != info.text_position) {
		SetDragDropFocus();
		RedrawDragDropCaretRegion();
		drag_drop_caret_position = info.text_position;
		drag_drop_caret_region = Rect(info.geometry_region.point, Size(drag_drop_caret_width, info.geometry_region.size.height));
		RedrawDragDropCaretRegion();
	}
}

void TextView::CancelDragDrop() {
	RedrawDragDropCaretRegion(); drag_drop_caret_region = region_empty;
}

TextView& TextView::MergeBackWith(TextView& text_view) {
	InsertText(text.size(), text_view.text);
	return *this;
}

void TextView::FinishDragDrop(BlockView& source) {
	TextView& text_view = static_cast<TextView&>(source);
	if (&text_view == this) {
		size_t selection_range_end = get_selection_range_end();
		if (drag_drop_caret_position < selection_range_begin) {
			std::rotate(text.begin() + drag_drop_caret_position, text.begin() + selection_range_begin, text.begin() + selection_range_end);
		} else {
			std::rotate(text.begin() + selection_range_begin, text.begin() + selection_range_end, text.begin() + drag_drop_caret_position);
			drag_drop_caret_position -= selection_range_length;
		}
		TextModified();
	} else {
		std::wstring str = text_view.text.substr(selection_range_begin, selection_range_length);
		InsertText(drag_drop_caret_position, str);
		text_view.DeleteText(selection_range_begin, selection_range_length);
	}
	UpdateSelectionRegion(drag_drop_caret_position, selection_range_length);
}

void TextView::Insert(wchar ch) {
	if (HasSelectionFocus()) {
		ReplaceText(selection_range_begin, selection_range_length, ch);
		SetCaret(selection_range_begin + 1);
	} else {
		InsertText(caret_position, ch);
		SetCaret(caret_position + 1);
	}
}

void TextView::Insert(std::wstring str) {
	if (HasSelectionFocus()) {
		ReplaceText(selection_range_begin, selection_range_length, str);
		SetCaret(selection_range_begin + str.length());
	} else {
		InsertText(caret_position, str);
		SetCaret(caret_position + str.length());
	}
}

void TextView::Split() {
	std::wstring str;
	if (HasSelectionFocus()) {
		str = text.substr(get_selection_range_end());
		DeleteText(selection_range_begin, -1);
	} else {
		str = text.substr(caret_position);
		DeleteText(caret_position, -1);
	}
	GetParent().InsertAfterSelfOrFront(str, IsCtrlDown()).SetCaret(0);
}

void TextView::Indent() {
	IsShiftDown() ? GetParent().IndentSelfShift() : GetParent().IndentSelf();
}

void TextView::Delete(bool is_backspace) {
	if (HasSelectionFocus()) {
		DeleteText(selection_range_begin, selection_range_length);
		SetCaret(selection_range_begin);
	} else {
		if (is_backspace) {
			if (caret_position == 0) {
				size_t reverse_pos = text.length();
				TextView& text_view = GetParent().MergeBeforeSelf();
				text_view.SetCaret(text_view.text.length() - reverse_pos);
			} else {
				size_t previous_caret_position = caret_position;
				SetCaret(caret_position - 1);
				DeleteText(caret_position, previous_caret_position - caret_position);
			}
		} else {
			if (caret_position >= text.length()) {
				size_t pos = text.length();
				GetParent().MergeAfterSelf().SetCaret(pos);
			} else {
				SetCaret(caret_position);
				DeleteText(caret_position, GetCharacterLength(caret_position));
			}
		}
	}
}

void TextView::Cut() {
	if (HasSelectionFocus()) {
		Copy();
		Delete(false);
	}
}

void TextView::Copy() {
	if (HasSelectionFocus()) {
		SetClipboardData(text.substr(selection_range_begin, selection_range_length));
	}
}

void TextView::Paste() {
	std::wstring str; GetClipboardData(str);
	std::vector<std::wstring> text_list = split_string_filtered(str);
	if (text_list.size() == 1) {
		Insert(text_list.front());
	} else {
		if (HasSelectionFocus()) {
			str = this->text.substr(get_selection_range_end());
			ReplaceText(selection_range_begin, -1, text_list.front());
		} else {
			str = this->text.substr(caret_position);
			ReplaceText(caret_position, -1, text_list.front());
		}
		text_list.erase(text_list.begin()); text_list.back().insert(0, str);
		GetParent().InsertAfterSelf(text_list).SetCaret(str.size());
	}
}

void TextView::OnImeBegin() {
	if (HasSelectionFocus()) {
		ime_composition_begin = selection_range_begin;
		ime_composition_length = selection_range_length;
	} else {
		ime_composition_begin = caret_position;
		ime_composition_length = 0;
	}
	ime.SetPosition(*this, caret_region.LeftBottom());
}

void TextView::OnImeString() {
	std::wstring str = ime.GetString();
	ReplaceText(ime_composition_begin, ime_composition_length, str);
	ime_composition_length = str.length();
	SetCaret(ime_composition_begin + ime.GetCursorPosition());
}

void TextView::OnImeEnd() {
	if (caret_position != get_ime_composition_end()) { SetCaret(get_ime_composition_end()); }
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
