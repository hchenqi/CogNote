#include "BlockPairView.h"
#include "BlockTextView.h"
#include "BlockListView.h"

#include "WndDesign/frame/PaddingFrame.h"


BlockPairView::BlockPairView(BlockView& parent, std::wstring text) :
	BlockView(parent),
	first(text_view = new BlockTextView(*this, text)),
	second(new PaddingFrame(Padding(20px, 1px, 0, 1px), list_view = new BlockListView(*this))) {
	RegisterChild(first); RegisterChild(second);
}

BlockListView& BlockPairView::GetParent() { return static_cast<BlockListView&>(BlockView::GetParent()); }

void BlockPairView::Load() {
	std::pair<block_ref, block_ref> data; block.read(data);
	LoadChild(*text_view, data.first); LoadChild(*list_view, data.second);
}

void BlockPairView::Save() {
	block.write(std::make_pair(GetChildRef(*text_view), GetChildRef(*list_view)));
}

Point BlockPairView::ConvertToListViewPoint(Point point) {
	return point_zero + (point - ConvertDescendentPoint(*list_view, point_zero));
}

Size BlockPairView::OnSizeRefUpdate(Size size_ref) {
	if (width != size_ref.width) {
		width = size_ref.width;
		length_first = UpdateChildSizeRef(first, Size(width, length_min)).height;
		length_second = UpdateChildSizeRef(second, Size(width, length_min)).height;
	}
	return GetSize();
}

void BlockPairView::OnChildSizeUpdate(WndObject& child, Size child_size) {
	if (&child == first.get()) {
		if (length_first != child_size.height) { length_first = child_size.height; SizeUpdated(GetSize()); }
	} else {
		if (length_second != child_size.height) { length_second = child_size.height; SizeUpdated(GetSize()); }
	}
}

Transform BlockPairView::GetChildTransform(WndObject& child) const {
	return GetChildRegion(child).point - point_zero;
}

void BlockPairView::OnChildRedraw(WndObject& child, Rect child_redraw_region) {
	Rect child_region = GetChildRegion(child);
	Redraw(child_region.Intersect(child_redraw_region + (child_region.point - point_zero)));
}

void BlockPairView::OnDraw(FigureQueue& figure_queue, Rect draw_region) {
	DrawChild(first, GetRegionFirst(), figure_queue, draw_region);
	DrawChild(second, GetRegionSecond(), figure_queue, draw_region);
}

void BlockPairView::SetCaret(Point point) {
	if (HitTestTextView(point)) {
		SetChildCaret(GetTextView(), point);
	} else {
		SetChildCaret(GetListView(), ConvertToListViewPoint(point));
	}
}

void BlockPairView::SetTextViewCaret(size_t pos) { GetTextView().SetCaret(pos); }

void BlockPairView::BeginSelect(BlockView& child) { is_text_view_selection_begin = &child == &GetTextView(); }

void BlockPairView::DoSelect(Point point) {
	if (is_text_view_selection_begin) {
		if (HitTestTextView(point)) {
			DoChildSelect(GetTextView(), point);
		} else {
			SelectSelf();
		}
	} else {
		if (HitTestTextView(point)) {
			SelectSelf();
		} else {
			DoChildSelect(GetListView(), ConvertToListViewPoint(point));
		}
	}
}

void BlockPairView::SelectChild(BlockView& child) { SelectSelf(); }

void BlockPairView::DoDragDrop(BlockView& source, Point point) {
	if (dynamic_cast<BlockListView*>(&source) == nullptr) {
		if (HitTestTextView(point)) {
			DoChildDragDrop(GetTextView(), source, point);
		} else {
			DoChildDragDrop(GetListView(), source, ConvertToListViewPoint(point));
		}
	} else {
		if (point.y < GetRegionFirst().Center().y) {
			GetParent().DoDragDropBefore(*this);
		} else {
			DoChildDragDrop(GetListView(), source, ConvertToListViewPoint(point));
		}
	}
}

BlockTextView& BlockPairView::InsertFront(std::wstring text) {
	return GetListView().InsertFront(text).GetTextView();
}

BlockTextView& BlockPairView::InsertFront(std::vector<std::wstring> text_list) {
	return GetListView().InsertFront(text_list).GetTextView();
}

BlockPairView& BlockPairView::InsertBack(std::unique_ptr<BlockPairView> pair_view) {
	return GetListView().InsertBack(std::move(pair_view));
}

BlockListView& BlockPairView::InsertBack(std::vector<std::unique_ptr<BlockPairView>> pair_view_list) {
	return GetListView().InsertBack(std::move(pair_view_list));
}

BlockTextView& BlockPairView::InsertAfterSelf(std::wstring text) {
	return GetParent().InsertAfter(*this, text).GetTextView();
}

BlockTextView& BlockPairView::InsertAfterSelf(std::vector<std::wstring> text_list) {
	return GetParent().InsertAfter(*this, text_list).GetTextView();
}

BlockPairView& BlockPairView::InsertAfterSelf(std::unique_ptr<BlockPairView> pair_view) {
	return GetParent().InsertAfter(*this, std::move(pair_view));
}

BlockListView& BlockPairView::InsertAfterSelf(std::vector<std::unique_ptr<BlockPairView>> pair_view_list) {
	return GetParent().InsertAfter(*this, std::move(pair_view_list));
}

BlockPairView& BlockPairView::MergeFront() {
	if (std::unique_ptr<BlockPairView> front = GetListView().PopFront(); front != nullptr) {
		GetTextView().MergeBackWith(front->GetTextView());
		GetListView().MergeFrontWith(front->GetListView());
	}
	return *this;
}

BlockPairView& BlockPairView::MergeWith(BlockPairView& pair_view) {
	GetTextView().MergeBackWith(pair_view.GetTextView());
	GetListView().MergeBackWith(pair_view.GetListView());
	return *this;
}

BlockTextView& BlockPairView::MergeBeforeSelf() {
	return GetParent().MergeBefore(*this).GetTextView();
}

BlockTextView& BlockPairView::MergeAfterSelf() {
	return GetParent().MergeAfter(*this).GetTextView();
}

BlockTextView& BlockPairView::IndentSelf() {
	return GetParent().Indent(*this).GetTextView();
}
