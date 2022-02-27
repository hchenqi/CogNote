#include "BlockListView.h"

#include "BlockPairView.h"

#include "WndDesign/figure/shape.h"


BEGIN_NAMESPACE(WndDesign)

BEGIN_NAMESPACE(Anonymous)

// style
constexpr size_t gap = 5;

// selection
constexpr Color selection_color = Color(Color::DimGray, 0x7f);

size_t selection_range_begin = 0;
size_t selection_range_end = 0;
Rect selection_region = region_empty;

// drag and drop
constexpr float drag_drop_caret_height = 1.0f;
constexpr Color drag_drop_caret_color = Color::DimGray;

size_t drag_drop_caret_position = 0;
Rect drag_drop_caret_region = region_empty;

END_NAMESPACE(Anonymous)


BlockPairView& BlockListView::GetParent() { return static_cast<BlockPairView&>(BlockView::GetParent()); }

void BlockListView::UpdateIndex(size_t begin) {
	for (size_t index = begin; index < child_list.size(); ++index) {
		SetChildIndex(child_list[index].child, index);
	}
}

BlockPairView& BlockListView::GetChild(child_ptr& child) { return static_cast<BlockPairView&>(*child); }

void BlockListView::InsertChild(size_t index, child_ptr child) {
	if (index > child_list.size()) { index = child_list.size(); }
	auto it = child_list.emplace(child_list.begin() + index, std::move(child));
	RegisterChild(it->child);
	it->length = UpdateChildSizeRef(it->child, Size(size.width, length_min)).height;
	UpdateIndex(index); UpdateLayout(index);
}

void BlockListView::InsertChild(size_t index, std::vector<child_ptr> children) {
	if (index > child_list.size()) { index = child_list.size(); }
	auto it = child_list.insert(child_list.begin() + index, std::make_move_iterator(children.begin()), std::make_move_iterator(children.end()));
	for (auto it_end = it + children.size(); it != it_end; ++it) {
		RegisterChild(it->child);
		it->length = UpdateChildSizeRef(it->child, Size(size.width, length_min)).height;
	}
	UpdateIndex(index); UpdateLayout(index);
}

void BlockListView::EraseChild(size_t begin, size_t count) {
	if (begin > child_list.size() || count == 0) { return; }
	size_t end = begin + count; if (end > child_list.size()) { end = child_list.size(); }
	child_list.erase(child_list.begin() + begin, child_list.begin() + end);
	UpdateIndex(begin); UpdateLayout(begin);
}

BlockListView::child_iter BlockListView::HitTestItem(float offset) {
	if (offset < 0.0f) { return child_list.begin(); }
	static auto cmp = [](const ChildInfo& item, float offset) { return offset >= item.offset; };
	return std::lower_bound(child_list.begin(), child_list.end(), offset, cmp) - 1;
}

void BlockListView::UpdateLayout(size_t index) {
	size.height = index == 0 ? (child_list.size() == 0 ? 0.0f : 0 - gap) : child_list[index - 1].EndOffset();
	for (index; index < child_list.size(); index++) {
		size.height += gap;
		child_list[index].offset = size.height;
		size.height += child_list[index].length;
	}
	SizeUpdated(size);
}

Size BlockListView::OnSizeRefUpdate(Size size_ref) {
	if (size.width != size_ref.width) {
		size.width = size_ref.width;
		size.height = 0.0f;
		for (auto& info : child_list) {
			info.offset = size.height;
			info.length = UpdateChildSizeRef(*info.child, Size(size.width, length_min)).height;
			size.height += info.length + gap;
		}
		size.height -= child_list.empty() ? 0.0f : gap;
		if (HasSelectionFocus()) {
			UpdateSelectionRegion(selection_range_begin, selection_range_end);
		}
	}
	return size;
}

void BlockListView::OnChildSizeUpdate(WndObject& child, Size child_size) {
	size_t index = GetChildIndex(child);
	if (child_list[index].length != child_size.height) {
		child_list[index].length = child_size.height;
		UpdateLayout(index + 1);
	}
}

Transform BlockListView::GetChildTransform(WndObject& child) const {
	return GetChildRegion(child).point - point_zero;
}

void BlockListView::OnChildRedraw(WndObject& child, Rect child_redraw_region) {
	Rect child_region = GetChildRegion(child);
	Redraw(child_region.Intersect(child_redraw_region + (child_region.point - point_zero)));
}

void BlockListView::OnDraw(FigureQueue& figure_queue, Rect draw_region) {
	draw_region = draw_region.Intersect(Rect(point_zero, size)); if (draw_region.IsEmpty()) { return; }
	auto it_begin = HitTestItem(draw_region.top());
	auto it_end = HitTestItem(ceilf(draw_region.bottom()) - 1.0f);
	for (auto it = it_begin; it <= it_end; ++it) {
		Rect child_region(Point(0.0f, it->offset), Size(size.width, it->length));
		DrawChild(it->child, child_region, figure_queue, draw_region);
	}
	if (HasSelectionFocus()) {
		figure_queue.add(selection_region.point, new Rectangle(selection_region.size, selection_color));
	}
	if (HasDragDropFocus()) {
		figure_queue.add(drag_drop_caret_region.point, new Rectangle(drag_drop_caret_region.size, drag_drop_caret_color));
	}
}

void BlockListView::SetCaret(Point point) {
	if (child_list.empty()) { return GetParent().SetTextViewCaret(-1); }
	auto it = HitTestItem(point.y); point.y -= it->offset;
	SetChildCaret(GetChild(it->child), point);
}

void BlockListView::RedrawSelectionRegion() { Redraw(selection_region); }

void BlockListView::UpdateSelectionRegion(size_t begin, size_t end) {
	SetSelectionFocus();
	RedrawSelectionRegion();
	selection_range_begin = begin; selection_range_end = end;
	if (end == begin) { selection_region = region_empty; return; }
	selection_region.point.x = 0.0f;
	selection_region.point.y = child_list[begin].BeginOffset();
	selection_region.size.width = size.width;
	selection_region.size.height = child_list[end - 1].EndOffset() - selection_region.point.y;
	RedrawSelectionRegion();
}

bool BlockListView::HitTestSelection(Point point) { return selection_region.Contains(point); }

void BlockListView::BeginSelect(BlockView& child) { selection_begin = GetChildIndex(child); }

void BlockListView::DoSelect(Point point) {
	if (child_list.empty()) { return; }
	auto it = HitTestItem(point.y);
	size_t index = it - child_list.begin();
	if (selection_begin == index) {
		point.y -= it->offset;
		DoChildSelect(GetChild(it->child), point);
	} else {
		size_t begin = selection_begin, end = index; if (end < begin) { std::swap(begin, end); }
		if (selection_range_begin == begin && selection_range_end == end + 1) { return; }
		UpdateSelectionRegion(begin, end + 1);
	}
}

void BlockListView::SelectChild(BlockView& child) {
	size_t index = GetChildIndex(child);
	UpdateSelectionRegion(index, index + 1);
}

void BlockListView::SelectMore() {
	if (selection_range_begin == 0 && selection_range_end == child_list.size()) { return SelectSelf(); }
	UpdateSelectionRegion(0, child_list.size());
}

void BlockListView::ClearSelection() {
	RedrawSelectionRegion(); selection_region = region_empty;
	selection_range_begin = selection_range_end = 0;
}

void BlockListView::RedrawDragDropCaretRegion() { Redraw(drag_drop_caret_region); }

void BlockListView::DoDragDrop(BlockView& source, Point point) {
	if (dynamic_cast<BlockListView*>(&source) == nullptr) {
		if (child_list.empty()) { return ClearDragDropFocus(); }
		auto it = HitTestItem(point.y); point.y -= it->offset;
		return DoChildDragDrop(GetChild(it->child), source, point);
	} else {
		size_t caret_position = 0;
		if (!child_list.empty()) {
			auto it = HitTestItem(point.y); point.y -= it->offset;
			caret_position = point.y < 0.0f ? 0 : it - child_list.begin() + 1;
			if (HasSelectionFocus() && selection_range_begin <= caret_position && caret_position <= selection_range_end) { return ClearDragDropFocus(); }
			if (point.y >= 0.0f && point.y < it->length) { return DoChildDragDrop(GetChild(it->child), source, point); }
		}
		if (!HasDragDropFocus() || caret_position != drag_drop_caret_position) {
			SetDragDropFocus();
			RedrawDragDropCaretRegion();
			drag_drop_caret_position = caret_position;
			drag_drop_caret_region = caret_position == 0 ?
				Rect(Point(0.0f, -drag_drop_caret_height), Size(size.width, drag_drop_caret_height)) :
				Rect(Point(0.0f, child_list[caret_position - 1].EndOffset()), Size(size.width, drag_drop_caret_height));
			RedrawDragDropCaretRegion();
		}
	}
}

void BlockListView::CancelDragDrop() {
	RedrawDragDropCaretRegion(); drag_drop_caret_region = region_empty;
}

void BlockListView::FinishDragDrop(BlockView& source) {
	BlockListView& list_view = static_cast<BlockListView&>(source);
	size_t selection_length = selection_range_end - selection_range_begin;
	if (&list_view == this && drag_drop_caret_position > selection_range_end) { drag_drop_caret_position -= selection_length; }
	InsertAt(drag_drop_caret_position, list_view.Extract(selection_range_begin, selection_range_end));
	UpdateSelectionRegion(drag_drop_caret_position, drag_drop_caret_position + selection_length);
}

void BlockListView::InsertAt(size_t index, std::wstring text) {
	InsertChild(index, new BlockPairView(*this, text));
	GetChild(index).SetTextViewCaret(0);
}

void BlockListView::InsertAt(size_t index, std::vector<std::wstring> text, size_t caret_pos) {
	std::vector<child_ptr> children; children.reserve(text.size());
	for (auto it = text.begin(); it != text.end(); ++it) {
		children.emplace_back(new BlockPairView(*this, std::move(*it)));
	}
	InsertChild(index, std::move(children));
	GetChild(index + text.size() - 1).SetTextViewCaret(caret_pos);
}

void BlockListView::InsertAt(size_t index, std::vector<std::unique_ptr<BlockPairView>> pair_view_list) {
	size_t end = index + pair_view_list.size();
	for (auto& it : pair_view_list) { SetChildParent(*it); }
	InsertChild(index, std::move(reinterpret_cast<std::vector<child_ptr>&>(pair_view_list)));
	UpdateSelectionRegion(index, end);
}

std::vector<std::unique_ptr<BlockPairView>> BlockListView::Extract(size_t begin, size_t end) {
	std::vector<std::unique_ptr<BlockPairView>> pair_view_list; pair_view_list.reserve(end - begin);
	for (size_t i = begin; i < end; ++i) {
		UnregisterChild(child_list[i].child);
		pair_view_list.emplace_back(static_cast<alloc_ptr<BlockPairView>>(child_list[i].child.release()));
	}
	EraseChild(begin, end - begin);
	ClearSelection();
	return pair_view_list;
}

void BlockListView::Delete() {
	EraseChild(selection_range_begin, selection_range_end - selection_range_begin);
	ClearSelection();
}

void BlockListView::Indent() {
	if (IsShiftDown()) {
		if (GetParent().IsRoot()) { return; }
		GetParent().InsertAfterSelf(Extract(selection_range_begin, selection_range_end));
	} else {
		if (selection_range_begin == 0) { return; }
		GetChild(selection_range_begin - 1).InsertBack(Extract(selection_range_begin, selection_range_end));
	}
}

void BlockListView::OnKeyMsg(KeyMsg msg) {
	switch (msg.type) {
	case KeyMsg::KeyDown:
		switch (msg.key) {
		case Key::Backspace:
		case Key::Delete: Delete(); break;

		case Key::Tab: Indent(); break;
		}
		break;
	}
}


END_NAMESPACE(WndDesign)