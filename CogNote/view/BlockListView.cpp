#include "BlockListView.h"
#include "BlockPairView.h"

#include "BlockStore/layout_traits_stl.h"

#include "WndDesign/figure/shape.h"


BEGIN_NAMESPACE(Anonymous)

// style
constexpr float gap = 1.0f;

// selection
constexpr Color selection_color = Color(Color::DimGray, 0x7f);

size_t selection_range_begin = 0;
size_t selection_range_length = 0;
Rect selection_region = region_empty;

inline size_t get_selection_range_end() { return selection_range_begin + selection_range_length; }
inline bool PositionInSelection(size_t pos) { return selection_range_begin <= pos && pos < get_selection_range_end(); }
inline bool PositionCoveredBySelection(size_t pos) { return selection_range_begin <= pos && pos <= get_selection_range_end(); }

// drag and drop
constexpr float drag_drop_caret_height = 1.0f;
constexpr Color drag_drop_caret_color = Color::DimGray;

size_t drag_drop_caret_position = 0;
Rect drag_drop_caret_region = region_empty;

END_NAMESPACE(Anonymous)


BlockPairView& BlockListView::GetParent() { return static_cast<BlockPairView&>(BlockView::GetParent()); }

void BlockListView::Load() {
	std::vector<block_ref> data; block.read(data);
	child_list.clear(); child_list.reserve(data.size()); size_t index = 0;
	for (auto& it : data) {
		auto& info = child_list.emplace_back(new BlockPairView(*this));
		LoadChild(GetChild(info.child), it);
		RegisterChild(info.child); SetChildIndex(info.child, index++);
		info.length = UpdateChildSizeRef(info.child, Size(size.width, length_min)).height;
	}
	UpdateLayout(0);
}

void BlockListView::Save() {
	std::vector<block_ref> data; data.reserve(child_list.size());
	for (auto& it : child_list) { data.push_back(GetChildRef(GetChild(it.child))); }
	block.write(data);
}

BlockPairView& BlockListView::GetChild(child_ptr& child) { return static_cast<BlockPairView&>(*child); }

void BlockListView::UpdateIndex(size_t begin) {
	for (size_t index = begin; index < child_list.size(); ++index) {
		SetChildIndex(child_list[index].child, index);
	}
	DataModified();
}

BlockListView::child_iter BlockListView::HitTestItem(float offset) {
	if (offset < 0.0f) { return child_list.begin(); }
	static auto cmp = [](const ChildInfo& item, float offset) { return offset >= item.offset; };
	return std::lower_bound(child_list.begin(), child_list.end(), offset, cmp) - 1;
}

void BlockListView::UpdateLayout(size_t index) {
	size.height = index == 0 ? (child_list.size() == 0 ? 0.0f : -gap) : child_list[index - 1].EndOffset();
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
			UpdateSelectionRegion(selection_range_begin, selection_range_length);
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
	if (!child_list.empty()) {
		auto it_begin = HitTestItem(draw_region.top()), it_end = HitTestItem(ceilf(draw_region.bottom()) - 1.0f);
		for (auto it = it_begin; it <= it_end; ++it) {
			Rect child_region(Point(0.0f, it->offset), Size(size.width, it->length));
			DrawChild(it->child, child_region, figure_queue, draw_region);
		}
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

void BlockListView::UpdateSelectionRegion(size_t begin, size_t length) {
	SetSelectionFocus();
	RedrawSelectionRegion();
	selection_range_begin = begin; selection_range_length = length;
	if (length == 0) { selection_region = region_empty; return; }
	selection_region.point.x = 0.0f;
	selection_region.point.y = child_list[begin].BeginOffset();
	selection_region.size.width = size.width;
	selection_region.size.height = child_list[begin + length - 1].EndOffset() - selection_region.point.y;
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
		size_t length = end + 1 - begin;
		if (selection_range_begin == begin && selection_range_length == length) { return; }
		UpdateSelectionRegion(begin, length);
	}
}

void BlockListView::SelectChild(BlockView& child) { UpdateSelectionRegion(GetChildIndex(child), 1); }

void BlockListView::SelectMore() {
	if (selection_range_begin == 0 && selection_range_length == child_list.size()) { return SelectSelf(); }
	UpdateSelectionRegion(0, child_list.size());
}

void BlockListView::ClearSelection() { RedrawSelectionRegion(); }

void BlockListView::RedrawDragDropCaretRegion() { Redraw(drag_drop_caret_region); }

void BlockListView::UpdateDragDropCaretRegion(size_t pos) {
	if (HasSelectionFocus() && PositionCoveredBySelection(pos)) { return ClearDragDropFocus(); }
	if (HasDragDropFocus() && pos == drag_drop_caret_position) { return; }
	SetDragDropFocus();
	RedrawDragDropCaretRegion();
	drag_drop_caret_position = pos;
	drag_drop_caret_region = pos == 0 ?
		Rect(Point(0.0f, -drag_drop_caret_height), Size(size.width, drag_drop_caret_height)) :
		Rect(Point(0.0f, child_list[pos - 1].EndOffset()), Size(size.width, drag_drop_caret_height));
	RedrawDragDropCaretRegion();
}

void BlockListView::DoDragDrop(BlockView& source, Point point) {
	if (dynamic_cast<BlockListView*>(&source) == nullptr) {
		if (child_list.empty()) { return ClearDragDropFocus(); }
		auto it = HitTestItem(point.y); point.y -= it->offset;
		return DoChildDragDrop(GetChild(it->child), source, point);
	} else {
		if (child_list.empty() || point.y < 0.0f) { return UpdateDragDropCaretRegion(0); }
		auto it = HitTestItem(point.y); size_t pos = it - child_list.begin();
		if (HasSelectionFocus() && PositionInSelection(pos)) { return ClearDragDropFocus(); }
		if (point.y -= it->offset; point.y < it->length) { return DoChildDragDrop(GetChild(it->child), source, point); }
		return UpdateDragDropCaretRegion(pos + 1);
	}
}

void BlockListView::CancelDragDrop() {
	RedrawDragDropCaretRegion(); drag_drop_caret_region = region_empty;
}

void BlockListView::InsertChild(size_t index, child_ptr child) {
	RegisterChild(child);
	if (index > child_list.size()) { index = child_list.size(); }
	auto it = child_list.emplace(child_list.begin() + index, std::move(child));
	UpdateIndex(index);
	it->length = UpdateChildSizeRef(it->child, Size(size.width, length_min)).height;
	UpdateLayout(index);
}

void BlockListView::InsertChild(size_t index, std::vector<child_ptr> children) {
	for (auto& child : children) { RegisterChild(child); }
	if (index > child_list.size()) { index = child_list.size(); }
	auto it = child_list.insert(child_list.begin() + index, std::make_move_iterator(children.begin()), std::make_move_iterator(children.end()));
	UpdateIndex(index);
	for (auto it_end = it + children.size(); it != it_end; ++it) {
		it->length = UpdateChildSizeRef(it->child, Size(size.width, length_min)).height;
	}
	UpdateLayout(index);
}

void BlockListView::EraseChild(size_t begin, size_t length) {
	if (begin > child_list.size() || length == 0) { return; }
	size_t end = begin + length; if (end > child_list.size()) { end = child_list.size(); }
	child_list.erase(child_list.begin() + begin, child_list.begin() + end);
	UpdateIndex(begin); UpdateLayout(begin);
}

BlockPairView& BlockListView::InsertChild(size_t index, std::wstring text) {
	InsertChild(index, new BlockPairView(*this, text));
	return GetChild(index);
}

BlockPairView& BlockListView::InsertChild(size_t index, std::vector<std::wstring> text_list) {
	std::vector<child_ptr> children; children.reserve(text_list.size());
	for (auto it = text_list.begin(); it != text_list.end(); ++it) {
		children.emplace_back(new BlockPairView(*this, std::move(*it)));
	}
	InsertChild(index, std::move(children));
	return GetChild(index + text_list.size() - 1);
}

BlockPairView& BlockListView::InsertChild(size_t index, std::unique_ptr<BlockPairView> pair_view) {
	SetChildParent(*pair_view);
	InsertChild(index, child_ptr(std::move(pair_view)));
	return GetChild(index);
}

void BlockListView::InsertChild(size_t index, std::vector<std::unique_ptr<BlockPairView>> pair_view_list) {
	for (auto& it : pair_view_list) { SetChildParent(*it); }
	InsertChild(index, std::move(reinterpret_cast<std::vector<child_ptr>&>(pair_view_list)));
}

std::unique_ptr<BlockPairView> BlockListView::ExtractChild(size_t index) {
	UnregisterChild(child_list[index].child);
	std::unique_ptr<BlockPairView> item(static_cast<alloc_ptr<BlockPairView>>(child_list[index].child.release()));
	EraseChild(index, 1);
	return item;
}

std::vector<std::unique_ptr<BlockPairView>> BlockListView::ExtractChild(size_t begin, size_t length) {
	std::vector<std::unique_ptr<BlockPairView>> pair_view_list; pair_view_list.reserve(length);
	for (size_t i = begin, end = begin + length; i < end; ++i) {
		UnregisterChild(child_list[i].child);
		pair_view_list.emplace_back(static_cast<alloc_ptr<BlockPairView>>(child_list[i].child.release()));
	}
	EraseChild(begin, length);
	return pair_view_list;
}

BlockPairView& BlockListView::Indent(size_t index) {
	if (IsShiftDown()) {
		return GetParent().IsRoot() ? GetChild(index) : GetParent().InsertAfterSelf(ExtractChild(index));
	} else {
		return index == 0 ? GetChild(0) : GetChild(index - 1).InsertBack(ExtractChild(index));
	}
}

BlockPairView& BlockListView::MergeBefore(size_t index) {
	return index == 0 ? GetParent().MergeFront() : MergeAfter(index - 1);
}

BlockPairView& BlockListView::MergeAfter(size_t index) {
	if (index >= child_list.size() - 1) { return GetParent(); }
	GetChild(index).MergeWith(GetChild(index + 1)); EraseChild(index + 1, 1);
	return GetChild(index);
}

void BlockListView::MergeAtWith(size_t index, BlockListView& list_view) {
	InsertChild(index, list_view.ExtractChild(0, list_view.child_list.size()));
}

void BlockListView::FinishDragDrop(BlockView& source) {
	ClearSelectionFocus();
	BlockListView& list_view = static_cast<BlockListView&>(source);
	if (&list_view == this && drag_drop_caret_position > selection_range_begin) { drag_drop_caret_position -= selection_range_length; }
	InsertChild(drag_drop_caret_position, list_view.ExtractChild(selection_range_begin, selection_range_length));
	UpdateSelectionRegion(drag_drop_caret_position, selection_range_length);
}

void BlockListView::Delete() {
	EraseChild(selection_range_begin, selection_range_length);
	if (selection_range_begin >= child_list.size()) {
		if (selection_range_begin == 0) {
			GetParent().SetTextViewCaret(-1);
		} else {
			GetChild(selection_range_begin - 1).SetTextViewCaret(-1);
		}
	} else {
		GetChild(selection_range_begin).SetTextViewCaret(0);
	}
}

void BlockListView::Indent() {
	if (IsShiftDown()) {
		if (GetParent().IsRoot()) { return; }
		ClearSelectionFocus();
		BlockListView& list_view = GetParent().InsertAfterSelf(ExtractChild(selection_range_begin, selection_range_length));
		list_view.UpdateSelectionRegion(list_view.GetChildIndex(GetParent()) + 1, selection_range_length);
	} else {
		if (selection_range_begin == 0) { return; }
		ClearSelectionFocus();
		BlockListView& list_view = GetChild(selection_range_begin - 1).InsertBack(ExtractChild(selection_range_begin, selection_range_length));
		list_view.UpdateSelectionRegion(list_view.child_list.size() - selection_range_length, selection_range_length);
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
