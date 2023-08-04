#include "PairView.h"
#include "TextView.h"
#include "ListView.h"

#include "WndDesign/frame/PaddingFrame.h"


PairView::PairView(BlockView& parent, std::wstring text) :
	BlockView(parent),
	first(text_view = new TextView(*this, text)),
	second(new PaddingFrame(Padding(20px, 1px, 0, 1px), list_view = new ListView(*this))) {
	RegisterChild(first); RegisterChild(second);
}

ListView& PairView::GetParent() { return static_cast<ListView&>(BlockView::GetParent()); }

void PairView::Load() {
	std::vector<block_ref> data = block.read().second; data.resize(2);
	LoadChild(*text_view, data[0]); LoadChild(*list_view, data[1]);
}

void PairView::Save() {
	block.write({}, { GetChildRef(*text_view), GetChildRef(*list_view) });
}

Point PairView::ConvertToListViewPoint(Point point) {
	return point_zero + (point - ConvertDescendentPoint(*list_view, point_zero));
}

Size PairView::OnSizeRefUpdate(Size size_ref) {
	if (width != size_ref.width) {
		width = size_ref.width;
		length_first = UpdateChildSizeRef(first, Size(width, length_min)).height;
		length_second = UpdateChildSizeRef(second, Size(width, length_min)).height;
	}
	return GetSize();
}

void PairView::OnChildSizeUpdate(WndObject& child, Size child_size) {
	if (&child == first.get()) {
		if (length_first != child_size.height) { length_first = child_size.height; SizeUpdated(GetSize()); }
	} else {
		if (length_second != child_size.height) { length_second = child_size.height; SizeUpdated(GetSize()); }
	}
}

Transform PairView::GetChildTransform(WndObject& child) const {
	return GetChildRegion(child).point - point_zero;
}

void PairView::OnChildRedraw(WndObject& child, Rect child_redraw_region) {
	Rect child_region = GetChildRegion(child);
	Redraw(child_region.Intersect(child_redraw_region + (child_region.point - point_zero)));
}

void PairView::OnDraw(FigureQueue& figure_queue, Rect draw_region) {
	DrawChild(first, GetRegionFirst(), figure_queue, draw_region);
	DrawChild(second, GetRegionSecond(), figure_queue, draw_region);
}

void PairView::SetCaret(Point point) {
	if (HitTestTextView(point)) {
		SetChildCaret(GetTextView(), point);
	} else {
		SetChildCaret(GetListView(), ConvertToListViewPoint(point));
	}
}

void PairView::SetTextViewCaret(size_t pos) { GetTextView().SetCaret(pos); }

void PairView::BeginSelect(BlockView& child) { is_text_view_selection_begin = &child == &GetTextView(); }

void PairView::DoSelect(Point point) {
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

void PairView::SelectChild(BlockView& child) { SelectSelf(); }

void PairView::DoDragDrop(BlockView& source, Point point) {
	if (dynamic_cast<ListView*>(&source) == nullptr) {
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

PairView& PairView::InsertBack(std::unique_ptr<PairView> pair_view) {
	return GetListView().InsertBack(std::move(pair_view));
}

PairView& PairView::InsertAfterSelf(std::unique_ptr<PairView> pair_view) {
	return GetParent().InsertAfter(*this, std::move(pair_view));
}

PairView& PairView::MergeWith(std::unique_ptr<PairView> pair_view) {
	GetTextView().MergeBackWith(pair_view->GetTextView());
	GetListView().MergeFrontWith(pair_view->GetListView());
	return *this;
}

PairView& PairView::InsertBackOrMergeWith(std::unique_ptr<PairView> pair_view) {
	if (GetListView().IsEmpty()) {
		return MergeWith(std::move(pair_view));
	} else {
		return InsertBack(std::move(pair_view));
	}
}

PairView& PairView::IndentAfterSelf() {
	return GetParent().IndentAfterChild(*this);
}

TextView& PairView::InsertAfterSelfOrFront(std::wstring text, bool ctrl) {
	if (GetListView().IsEmpty() ^ ctrl) {
		return GetParent().InsertAfter(*this, text).GetTextView();
	} else {
		return GetListView().InsertFront(text).GetTextView();
	}
}

TextView& PairView::IndentSelf() {
	return GetParent().IndentChild(*this).GetTextView();
}

TextView& PairView::IndentSelfShift() {
	return GetParent().IndentChildShift(*this).GetTextView();
}

TextView& PairView::MergeBeforeSelf() {
	return GetParent().MergeBeforeChild(*this).GetTextView();
}

TextView& PairView::MergeAfterSelf() {
	if (GetListView().IsEmpty()) {
		return GetParent().MergeAfterChild(*this).GetTextView();
	} else {
		return GetListView().MergeFrontChild().GetTextView();
	}
}

TextView& PairView::InsertAfterSelf(std::vector<std::wstring> text_list) {
	return GetParent().InsertAfter(*this, text_list).GetTextView();
}

ListView& PairView::InsertBack(std::vector<std::unique_ptr<PairView>> pair_view_list) {
	return GetListView().InsertBack(std::move(pair_view_list));
}

ListView& PairView::InsertAfterSelf(std::vector<std::unique_ptr<PairView>> pair_view_list) {
	return GetParent().InsertAfter(*this, std::move(pair_view_list));
}
