#include "BlockPairView.h"

#include "BlockTextView.h"
#include "BlockListView.h"

#include "WndDesign/frame/PaddingFrame.h"


BEGIN_NAMESPACE(WndDesign)


BlockPairView::BlockPairView(RootFrame& root) : BlockView(root) { Initialize(L"Root"); }

BlockPairView::BlockPairView(BlockView& parent, std::wstring text) : BlockView(parent) { Initialize(text); }

BlockListView& BlockPairView::GetParent() { return static_cast<BlockListView&>(BlockView::GetParent()); }

void BlockPairView::Initialize(std::wstring text) {
	first = text_view = new BlockTextView(*this, text);
	second = new PaddingFrame(Padding(20px, 1px, 0, 1px), list_view = new BlockListView(*this));
	RegisterChild(this->first); RegisterChild(this->second);
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

void BlockPairView::SetTextViewCaret(size_t pos) { GetTextView().SetCaret(pos); }

void BlockPairView::SetCaret(Point point) {
	if (HitTestTextView(point)) {
		SetChildCaret(GetTextView(), point);
	} else {
		SetChildCaret(GetListView(), ConvertToListViewPoint(point));
	}
}

void BlockPairView::BeginSelect(BlockView& child) { is_text_view_selection_begin = &child == &GetTextView(); }

void BlockPairView::DoSelect(Point point) {
	if (IsRoot()) {
		if (is_text_view_selection_begin) {
			DoChildSelect(GetTextView(), point);
		} else {
			DoChildSelect(GetListView(), ConvertToListViewPoint(point));
		}
	} else {
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
		if (point.y < GetRegionFirst().Center().y && !IsRoot()) {
			GetParent().DoDragDropBefore(*this);
		} else {
			DoChildDragDrop(GetListView(), source, ConvertToListViewPoint(point));
		}
	}
}

void BlockPairView::InsertFront(std::wstring text) {
	GetListView().InsertFront(text);
}

void BlockPairView::InsertBack(std::vector<std::unique_ptr<BlockPairView>> pair_view_list) {
	GetListView().InsertBack(std::move(pair_view_list));
}

void BlockPairView::InsertAfter(BlockView& child, std::wstring text) {
	GetListView().InsertAfter(child, text);
}

void BlockPairView::InsertAfter(BlockView& child, std::vector<std::wstring> text, size_t caret_pos) {
	GetListView().InsertAfter(child, text, caret_pos);
}

void BlockPairView::InsertAfter(BlockView& child, std::vector<std::unique_ptr<BlockPairView>> pair_view_list) {
	GetListView().InsertAfter(child, std::move(pair_view_list));
}

void BlockPairView::InsertAfterSelf(std::wstring text) {
	IsRoot() ? GetListView().InsertFront(text) : GetParent().InsertAfter(*this, text);
}

void BlockPairView::InsertAfterSelf(std::vector<std::wstring> text, size_t caret_pos) {
	IsRoot() ? GetListView().InsertFront(text, caret_pos) : GetParent().InsertAfter(*this, text, caret_pos);
}

void BlockPairView::InsertAfterSelf(std::vector<std::unique_ptr<BlockPairView>> pair_view_list) {
	IsRoot() ? void() : GetParent().InsertAfter(*this, std::move(pair_view_list));
}

void BlockPairView::MergeFront() {
	std::unique_ptr<BlockPairView> front = GetListView().PopFront(); if (front == nullptr) { return; }
	GetListView().MergeFrontWith(front->GetListView());
	GetTextView().MergeBackWith(front->GetTextView());
}

void BlockPairView::MergeWith(BlockPairView& pair_view) {
	GetListView().MergeBackWith(pair_view.GetListView());
	GetTextView().MergeBackWith(pair_view.GetTextView());
}

void BlockPairView::MergeBeforeSelf() {
	IsRoot() ? void() : GetParent().MergeBefore(*this);
}

void BlockPairView::MergeAfterSelf() {
	IsRoot() ? MergeFront() : GetParent().MergeAfter(*this);
}

void BlockPairView::IndentSelf() {
	IsRoot() ? void() : GetParent().Indent(*this);
}


END_NAMESPACE(WndDesign)