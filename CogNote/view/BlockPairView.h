#pragma once

#include "block_view.h"


class BlockTextView;
class BlockListView;


class BlockPairView : public BlockView, public LayoutType<Assigned, Auto> {
public:
	BlockPairView(BlockView& parent, std::wstring text = {});

	// context
private:
	BlockListView& GetParent();

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
	ref_ptr<BlockTextView> text_view;
	ref_ptr<BlockListView> list_view;
private:
	BlockTextView& GetTextView() { return *text_view; }
	BlockListView& GetListView() { return *list_view; }

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
	BlockTextView& InsertFront(std::wstring text);
	BlockTextView& InsertFront(std::vector<std::wstring> text_list);
	BlockPairView& InsertBack(std::unique_ptr<BlockPairView> pair_view);
	BlockListView& InsertBack(std::vector<std::unique_ptr<BlockPairView>> pair_view_list);
	BlockTextView& InsertAfterSelf(std::wstring text);
	BlockTextView& InsertAfterSelf(std::vector<std::wstring> text_list);
	BlockPairView& InsertAfterSelf(std::unique_ptr<BlockPairView> pair_view);
	BlockListView& InsertAfterSelf(std::vector<std::unique_ptr<BlockPairView>> pair_view_list);
	BlockPairView& MergeFront();
	BlockPairView& MergeWith(BlockPairView& pair_view);
	BlockTextView& MergeBeforeSelf();
	BlockTextView& MergeAfterSelf();
	BlockTextView& IndentSelf();
};
