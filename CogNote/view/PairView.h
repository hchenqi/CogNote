#pragma once

#include "block_view.h"


class TextView;
class ListView;


class PairView : public BlockView, public LayoutType<Assigned, Auto> {
public:
	PairView(BlockView& parent, std::wstring text = {});

	// context
private:
	ListView& GetParent();

	// data
private:
	virtual void Load() override;
	virtual void Save() override;

	// child
private:
	using child_ptr = child_ptr<Assigned, Auto>;
private:
	child_ptr first;
	child_ptr second;
private:
	ref_ptr<TextView> text_view;
	ref_ptr<ListView> list_view;
private:
	TextView& GetTextView() { return *text_view; }
	ListView& GetListView() { return *list_view; }

	// layout
private:
	float width = 0.0f;
	float length_first = 0.0f;
	float length_second = 0.0f;
private:
	Size GetSize() const { return Size(width, length_first + length_second); }
	Rect GetRegionFirst() const { return Rect(point_zero, Size(width, length_first)); }
	Rect GetRegionSecond() const { return Rect(Point(0.0f, length_first), Size(width, length_second)); }
	Rect GetChildRegion(WndObject& child) const { return &child == first.get() ? GetRegionFirst() : GetRegionSecond(); }
	bool HitTestTextView(Point point) { return point.y < length_first; }
	Point ConvertToListViewPoint(Point point);
private:
	virtual Size OnSizeRefUpdate(Size size_ref) override;
	virtual void OnChildSizeUpdate(WndObject& child, Size child_size) override;
private:
	virtual Transform GetChildTransform(WndObject& child) const override;

	// paint
private:
	virtual void OnChildRedraw(WndObject& child, Rect child_redraw_region) override;
	virtual void OnDraw(FigureQueue& figure_queue, Rect draw_region) override;

	// caret
private:
	virtual void SetCaret(Point point) override;
public:
	void SetTextViewCaret(size_t pos);

	// selection
private:
	bool is_text_view_selection_begin = false;
private:
	virtual void BeginSelect(BlockView& child) override;
	virtual void DoSelect(Point point) override;
	virtual void SelectChild(BlockView& child) override;

	// drag and drop
private:
	virtual void DoDragDrop(BlockView& source, Point point) override;

	// route
public:
	PairView& InsertBack(std::unique_ptr<PairView> pair_view);  // text indent
	PairView& InsertAfterSelf(std::unique_ptr<PairView> pair_view);  // text indent shift
	PairView& MergeWith(std::unique_ptr<PairView> pair_view);  // text backspace
	PairView& InsertBackOrMergeWith(std::unique_ptr<PairView> pair_view);  // text backspace
	PairView& IndentAfterSelf();  // text delete
public:
	TextView& InsertAfterSelf(std::wstring text);  // text split
	TextView& InsertFront(std::wstring text);  // text split ctrl
	TextView& IndentSelf();  // text indent
	TextView& IndentSelfShift();  // text indent shift
	TextView& MergeBeforeSelf();  // text backspace
	TextView& MergeAfterSelf();  // text delete
	TextView& InsertAfterSelf(std::vector<std::wstring> text_list);  // text paste
public:
	ListView& InsertBack(std::vector<std::unique_ptr<PairView>> pair_view_list);  // list indent
	ListView& InsertAfterSelf(std::vector<std::unique_ptr<PairView>> pair_view_list);  // list indent shift
};
