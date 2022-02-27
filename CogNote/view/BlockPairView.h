#pragma once

#include "block_view.h"


BEGIN_NAMESPACE(WndDesign)

class BlockTextView;
class BlockListView;


class BlockPairView : public BlockView, public LayoutType<Assigned, Auto> {
public:
	BlockPairView(RootFrame& root);
	BlockPairView(BlockView& parent, std::wstring text);

	// context
public:
	BlockView::IsRoot;
private:
	BlockListView& GetParent();

	// child
private:
	using child_ptr = child_ptr<Assigned, Auto>;
private:
	child_ptr first;
	child_ptr second;
private:
	ref_ptr<BlockTextView> text_view = nullptr;
	ref_ptr<BlockListView> list_view = nullptr;
private:
	void Initialize(std::wstring text);
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
public:
	void SetTextViewCaret(size_t pos);
private:
	virtual void SetCaret(Point point) override;

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

	// input
public:
	void InsertFront(std::wstring text);
	void InsertBack(std::vector<std::unique_ptr<BlockPairView>> pair_view_list);
	void InsertAfter(BlockView& child, std::wstring text);
	void InsertAfter(BlockView& child, std::vector<std::wstring> text, size_t caret_pos);
	void InsertAfter(BlockView& child, std::vector<std::unique_ptr<BlockPairView>> pair_view_list);
	void InsertAfterSelf(std::wstring text);
	void InsertAfterSelf(std::vector<std::wstring> text, size_t caret_pos);
	void InsertAfterSelf(std::vector<std::unique_ptr<BlockPairView>> pair_view_list);
};


END_NAMESPACE(WndDesign)