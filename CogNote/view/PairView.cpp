#include "PairView.h"
#include "TextView.h"
#include "ListView.h"

#include "WndDesign/frame/MaxFrame.h"
#include "WndDesign/frame/MinFrame.h"
#include "WndDesign/frame/CenterFrame.h"
#include "WndDesign/frame/PaddingFrame.h"


PairView::PairView(Block& parent, std::wstring text) :
	Block(parent), Base{
		new CenterFrame<Auto, Assigned>(new PaddingFrame(Padding(10px), new MaxFrame(size_max, text_view = new TextView(*this, text)))),
		new MinFrame(Size(30, 30), list_view = new ListView(*this))
	} {
}

ListView& PairView::GetParent() { return static_cast<ListView&>(Block::GetParent()); }

void PairView::Load() {
	std::vector<block_ref> data = block.read().second; data.resize(2);
	LoadChild(*text_view, data[0]); LoadChild(*list_view, data[1]);
}

void PairView::Save() {
	block.write({}, { GetChildRef(*text_view), GetChildRef(*list_view) });
}

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

void PairView::BeginSelect(Block& child) { is_text_view_selection_begin = &child == &GetTextView(); }

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

void PairView::SelectChild(Block& child) { SelectSelf(); }

void PairView::DoDragDrop(Block& source, Point point) {
	if (dynamic_cast<ListView*>(&source) == nullptr) {
		if (HitTestTextView(point)) {
			DoChildDragDrop(GetTextView(), source, ConvertToTextViewPoint(point));
		} else {
			DoChildDragDrop(GetListView(), source, ConvertToListViewPoint(point));
		}
	} else {
		if (HitTestTextView(point)) {
			if (point.y < GetRegionFirst().Center().y) {
				GetParent().DoDragDropBefore(*this);
			} else {
				GetParent().DoDragDropAfter(*this);
			}
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

TextView& PairView::InsertAfterSelf(std::vector<std::wstring> text_list) {
	return GetParent().InsertAfter(*this, text_list).GetTextView();
}

ListView& PairView::InsertBack(std::vector<std::unique_ptr<PairView>> pair_view_list) {
	return GetListView().InsertBack(std::move(pair_view_list));
}

ListView& PairView::InsertAfterSelf(std::vector<std::unique_ptr<PairView>> pair_view_list) {
	return GetParent().InsertAfter(*this, std::move(pair_view_list));
}
