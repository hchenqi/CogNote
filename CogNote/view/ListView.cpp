#include "ListView.h"
#include "PairView.h"

#include "WndDesign/system/clipboard.h"


PairView& ListView::GetParent() { return static_cast<PairView&>(Block::GetParent()); }

void ListView::Set(const value_type& value) {
	std::vector<child_ptr> children; children.reserve(value.size());
	for (auto& ref : value) {
		alloc_ptr<PairView> child = new PairView(*this, {});
		LoadChild(*child, ref);
		children.emplace_back(std::move(child));
	}
	Base::EraseChild(0, Length());
	Base::InsertChild(0, std::move(children));
}

ListView::value_type ListView::Get() { 
	std::vector<block_ref> data; data.reserve(Length());
	for (size_t index = 0; index < Length(); index++) {
		data.push_back(GetChildRef(GetChild(index)));
	}
	return data;
}

local_data ListView::GetLocalData(size_t begin, size_t length) const {
	local_data result;
	for (size_t index = begin; index < begin + length && index < Length(); ++index) {
		result.push_back(GetChild(index).GetLocalData());
	}
	return result;
}

PairView& ListView::GetChild(WndObject& child) const { return static_cast<PairView&>(child); }

void ListView::OnDraw(FigureQueue& figure_queue, Rect draw_region) {
	Base::OnDraw(figure_queue, draw_region);
	if (HasSelectionFocus()) {
		figure_queue.add(selection_region.point, new Rectangle(selection_region.size, selection_color));
	}
	if (HasDragDropFocus()) {
		figure_queue.add(drag_drop_caret_region.point, new Rectangle(drag_drop_caret_region.size, drag_drop_caret_color));
	}
}

void ListView::SetCaret(Point point) {
	if (Empty()) {
		if (IsRoot()) {
			InsertFront(L"").SetTextViewCaret(0);
		} else {
			GetParent().SetTextViewCaret(-1);
		}
	} else {
		size_t index = point.y <= 0.0 ? 0 : HitTestIndex(point);
		point -= GetChildRegion(index).point - point_zero;
		SetChildCaret(GetChild(index), point);
	}
}

void ListView::RedrawSelectionRegion() { Redraw(selection_region); }

void ListView::UpdateSelectionRegion(size_t begin, size_t length) {
	SetSelectionFocus();
	RedrawSelectionRegion();
	selection_range_begin = begin; selection_range_length = length;
	if (length == 0) { selection_region = region_empty; return; }
	selection_region = Rect(Interval(0, size.width), Interval::Union(GetChildRegion(begin).Vertical(), GetChildRegion(begin + length - 1).Vertical()));
	RedrawSelectionRegion();
}

bool ListView::HitTestSelection(Point point) { return selection_region.Contains(point); }

void ListView::BeginSelect(Block& child) { selection_begin = GetChildIndex(dynamic_cast<WndObject&>(child)); }

void ListView::DoSelect(Point point) {
	if (Empty()) { return; }
	size_t index = point.y <= 0.0 ? 0 : HitTestIndex(point);
	if (selection_begin == index) {
		point -= GetChildRegion(index).point - point_zero;
		DoChildSelect(GetChild(index), point);
	} else {
		size_t begin = selection_begin, end = index; if (end < begin) { std::swap(begin, end); }
		size_t length = end + 1 - begin;
		if (HasSelectionFocus() && selection_range_begin == begin && selection_range_length == length) { return; }
		UpdateSelectionRegion(begin, length);
	}
}

void ListView::SelectChild(Block& child) { UpdateSelectionRegion(GetChildIndex(dynamic_cast<WndObject&>(child)), 1); }

void ListView::SelectMore() {
	if (selection_range_begin == 0 && selection_range_length == Length()) { return SelectSelf(); }
	UpdateSelectionRegion(0, Length());
}

void ListView::ClearSelection() { RedrawSelectionRegion(); }

void ListView::RedrawDragDropCaretRegion() { Redraw(drag_drop_caret_region); }

void ListView::UpdateDragDropCaretRegion(size_t pos) {
	if (HasSelectionFocus() && PositionCoveredBySelection(pos)) { return ClearDragDropFocus(); }
	if (HasDragDropFocus() && pos == drag_drop_caret_position) { return; }
	SetDragDropFocus();
	RedrawDragDropCaretRegion();
	drag_drop_caret_position = pos;
	drag_drop_caret_region = pos == 0 ?
		Rect(Point(0.0f, -drag_drop_caret_height), Size(max(size.width, 10.0f), drag_drop_caret_height)) :
		Rect(Point(0.0f, GetChildRegion(pos - 1).bottom()), Size(size.width, drag_drop_caret_height));
	RedrawDragDropCaretRegion();
}

void ListView::DoDragDrop(Block& source, Point point) {
	if (dynamic_cast<ListView*>(&source) == nullptr) {
		if (Empty()) { return ClearDragDropFocus(); }
		size_t index = point.y <= 0.0 ? 0 : HitTestIndex(point);
		point -= GetChildRegion(index).point - point_zero;
		return DoChildDragDrop(GetChild(index), source, point);
	} else {
		if (Empty() || point.y < 0.0f) { return UpdateDragDropCaretRegion(0); }
		size_t index = point.y <= 0.0 ? 0 : HitTestIndex(point);
		if (HasSelectionFocus() && PositionInSelection(index)) { return ClearDragDropFocus(); }
		if (GetChildRegion(index).Contains(point)) {
			point -= GetChildRegion(index).point - point_zero;
			return DoChildDragDrop(GetChild(index), source, point);
		}
		return UpdateDragDropCaretRegion(index + 1);
	}
}

void ListView::CancelDragDrop() {
	RedrawDragDropCaretRegion(); drag_drop_caret_region = region_empty;
}

void ListView::FinishDragDrop(Block& source) {
	ClearSelectionFocus();
	ListView& list_view = static_cast<ListView&>(source);
	if (&list_view == this && drag_drop_caret_position > selection_range_begin) { drag_drop_caret_position -= selection_range_length; }
	InsertChild(drag_drop_caret_position, list_view.ExtractChild(list_view.selection_range_begin, list_view.selection_range_length));
	UpdateSelectionRegion(drag_drop_caret_position, list_view.selection_range_length);
}

PairView& ListView::InsertChild(size_t index, std::wstring text) {
	Base::InsertChild(index, new PairView(*this, { text }));
	DataModified();
	return GetChild(index);
}

PairView& ListView::InsertChild(size_t index, list_data text_list) {
	std::vector<child_ptr> children; children.reserve(text_list.size());
	for (auto& it : text_list) { children.emplace_back(new PairView(*this, std::move(it))); }
	Base::InsertChild(index, std::move(children));
	DataModified();
	return GetChild(index + text_list.size() - 1);
}

PairView& ListView::InsertChild(size_t index, std::unique_ptr<PairView> pair_view) {
	SetChildParent(*pair_view);
	Base::InsertChild(index, child_ptr(std::move(pair_view)));
	DataModified();
	return GetChild(index);
}

void ListView::InsertChild(size_t index, std::vector<std::unique_ptr<PairView>> pair_view_list) {
	std::vector<child_ptr> children; children.reserve(pair_view_list.size());
	for (auto& it : pair_view_list) { SetChildParent(*it); children.emplace_back(it.release()); }
	Base::InsertChild(index, std::move(children));
	DataModified();
}

std::unique_ptr<PairView> ListView::ExtractChild(size_t index) {
	std::unique_ptr<PairView> pair_view(static_cast<alloc_ptr<PairView>>(Base::ExtractChild(index).release()));
	DataModified();
	return pair_view;
}

std::vector<std::unique_ptr<PairView>> ListView::ExtractChild(size_t begin, size_t length) {
	std::vector<std::unique_ptr<PairView>> pair_view_list; pair_view_list.reserve(length);
	auto ptr_list = Base::ExtractChild(begin, length);
	for (auto& ptr : ptr_list) { pair_view_list.emplace_back(static_cast<alloc_ptr<PairView>>(ptr.release())); }
	DataModified();
	return pair_view_list;
}

PairView& ListView::InsertBack(std::unique_ptr<PairView> pair_view) {
	return InsertChild(Length(), std::move(pair_view));
}

PairView& ListView::InsertAfter(PairView& child, std::unique_ptr<PairView> pair_view) {
	return InsertChild(GetChildIndex(child) + 1, std::move(pair_view));
}

ListView& ListView::MergeFrontWith(ListView& list_view) {
	InsertChild(0, list_view.ExtractChild(0, list_view.Length()));
	return *this;
}

PairView& ListView::IndentAfterChild(PairView& child) {
	if (size_t index = GetChildIndex(child); index < Length() - 1) {
		return child.InsertBack(ExtractChild(index + 1));
	} else {
		if (!IsRoot()) { GetParent().IndentAfterSelf(); }
		return child;
	}
}

ListView& ListView::InsertBack(std::vector<std::unique_ptr<PairView>> pair_view_list) {
	InsertChild(Length(), std::move(pair_view_list));
	return *this;
}

ListView& ListView::InsertAfter(PairView& child, std::vector<std::unique_ptr<PairView>> pair_view_list) {
	InsertChild(GetChildIndex(child) + 1, std::move(pair_view_list));
	return *this;
}

PairView& ListView::InsertAfter(PairView& child, std::wstring text) {
	return InsertChild(GetChildIndex(child) + 1, text);
}

PairView& ListView::InsertFront(std::wstring text) {
	return InsertChild(0, text);
}

PairView& ListView::IndentChild(PairView& child) {
	size_t index = GetChildIndex(child); if (index == 0) { return child; }
	return GetChild(index - 1).InsertBack(ExtractChild(index));
}

PairView& ListView::IndentChildShift(PairView& child) {
	if (IsRoot()) { return child; }
	return GetParent().InsertAfterSelf(ExtractChild(GetChildIndex(child)));
}

PairView& ListView::MergeBeforeChild(PairView& child) {
	if (size_t index = GetChildIndex(child); index == 0) {
		return IsRoot() ? child : GetParent().MergeWith(ExtractChild(index));
	} else {
		return GetChild(index - 1).InsertBackOrMergeWith(ExtractChild(index));
	}
}

PairView& ListView::MergeFrontChild() {
	return GetParent().MergeWith(ExtractChild(0));
}

PairView& ListView::MergeAfterChild(PairView& child) {
	if (size_t index = GetChildIndex(child); index < Length() - 1) {
		return child.MergeWith(ExtractChild(index + 1));
	} else {
		if (!IsRoot()) { GetParent().IndentAfterSelf(); }
		return child;
	}
}

PairView& ListView::InsertAfter(PairView& child, list_data text_list) {
	return InsertChild(GetChildIndex(child) + 1, text_list);
}

void ListView::Delete() {
	ClearSelectionFocus();
	EraseChild(selection_range_begin, selection_range_length);
	DataModified();
	if (selection_range_begin >= Length()) {
		if (selection_range_begin == 0) {
			SetCaret(point_zero);
		} else {
			GetChild(selection_range_begin - 1).SetTextViewCaret(-1);
		}
	} else {
		GetChild(selection_range_begin).SetTextViewCaret(0);
	}
}

void ListView::Indent() {
	if (IsShiftDown()) {
		if (IsRoot()) { return; }
		ClearSelectionFocus();
		ListView& list_view = GetParent().InsertAfterSelf(ExtractChild(selection_range_begin, selection_range_length));
		list_view.UpdateSelectionRegion(list_view.GetChildIndex(GetParent()) + 1, selection_range_length);
	} else {
		if (selection_range_begin == 0) { return; }
		ClearSelectionFocus();
		ListView& list_view = GetChild(selection_range_begin - 1).InsertBack(ExtractChild(selection_range_begin, selection_range_length));
		list_view.UpdateSelectionRegion(list_view.Length() - selection_range_length, selection_range_length);
	}
}

void ListView::Cut() {
	Copy();
	Delete();
}

void ListView::Copy() {
	SetClipboardData(convert_to_string(GetLocalData(selection_range_begin, selection_range_length)));
}

void ListView::OnKeyMsg(KeyMsg msg) {
	switch (msg.type) {
	case KeyMsg::KeyDown:
		switch (msg.key) {
		case Key::Tab: Indent(); break;
		case Key::Backspace:
		case Key::Delete: Delete(); break;

		case CharKey('X'): if (IsCtrlDown()) { Cut(); } break;
		case CharKey('C'): if (IsCtrlDown()) { Copy(); } break;
		}
		break;
	}
}
