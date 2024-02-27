#include "PairView.h"
#include "TextView.h"
#include "ListView.h"

#include "WndDesign/frame/PaddingFrame.h"


PairView::PairView(BlockView& parent, pair_data data) :
	BlockView(parent), Base{
		child_ptr_first() = text_view = new TextView(*this, data.text),
		new PaddingFrame(Padding(20px, 1px, 0, 1px), list_view = new ListView(*this, data.list))
	} {
}

void PairView::Set(const value_type& value) {
	LoadBlock(*text_view, value.first); LoadBlock(*list_view, value.second);
}

PairView::value_type PairView::Get() {
	return { GetBlockRef(*text_view), GetBlockRef(*list_view) };
}

pair_data PairView::GetLocalData() const { return { text_view->GetLocalData(), list_view->GetLocalData() }; }

ListView& PairView::GetParent() { return static_cast<ListView&>(BlockView::GetParent()); }

Point PairView::ConvertToTextViewPoint(Point point) {
	return point_zero + (point - ConvertDescendentPoint(*text_view, point_zero));
}

Point PairView::ConvertToListViewPoint(Point point) {
	return point_zero + (point - ConvertDescendentPoint(*list_view, point_zero));
}

void PairView::OnDraw(FigureQueue& figure_queue, Rect draw_region) {
	Base::OnDraw(figure_queue, draw_region);
	if (text_view->IsModified() || list_view->IsModified()) {
		figure_queue.add(point_zero, new Rectangle(Size(1.0f, size.height), 1.0f, text_view->HasSaveError() || list_view->HasSaveError() ? color_error : color_unsaved));
	}
}

void PairView::SetCaret(Point point) {
	if (HitTestTextView(point)) {
		SetChildCaret(GetTextView(), ConvertToTextViewPoint(point));
	} else {
		SetChildCaret(GetListView(), ConvertToListViewPoint(point));
	}
}

void PairView::SetTextViewCaret(size_t pos) { GetTextView().SetCaret(pos); }

void PairView::BeginSelect(BlockView& child) { is_text_view_selection_begin = &child == &GetTextView(); }

void PairView::DoSelect(Point point) {
	if (is_text_view_selection_begin) {
		if (HitTestTextView(point)) {
			DoChildSelect(GetTextView(), ConvertToTextViewPoint(point));
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
			DoChildDragDrop(GetTextView(), source, ConvertToTextViewPoint(point));
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
	if (GetListView().Empty()) {
		return MergeWith(std::move(pair_view));
	} else {
		return InsertBack(std::move(pair_view));
	}
}

PairView& PairView::IndentAfterSelf() {
	return GetParent().IndentAfterChild(*this);
}

TextView& PairView::InsertAfterSelfOrFront(std::wstring text, bool ctrl) {
	if (GetListView().Empty() ^ ctrl) {
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
	if (GetListView().Empty()) {
		return GetParent().MergeAfterChild(*this).GetTextView();
	} else {
		return GetListView().MergeFrontChild().GetTextView();
	}
}

TextView& PairView::InsertAfterSelf(list_data text_list) {
	return GetParent().InsertAfter(*this, text_list).GetTextView();
}

ListView& PairView::InsertBack(std::vector<std::unique_ptr<PairView>> pair_view_list) {
	return GetListView().InsertBack(std::move(pair_view_list));
}

ListView& PairView::InsertAfterSelf(std::vector<std::unique_ptr<PairView>> pair_view_list) {
	return GetParent().InsertAfter(*this, std::move(pair_view_list));
}
