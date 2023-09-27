#pragma once

#include "block.h"
#include "local_data.h"
#include "block_view.h"

#include "WndDesign/layout/SplitLayout.h"
#include "WndDesign/figure/shape.h"


class TextView;
class ListView;


class PairView : public Block<std::pair<block_ref, block_ref>>, public BlockView, public SplitLayoutVertical<Assigned, Auto, Assigned, Auto> {
private:
	using Base = SplitLayoutVertical;

public:
	PairView(BlockView& parent, pair_data data);

	// data
private:
	virtual void Set(const value_type& value) override;
	virtual value_type Get() override;
public:
	pair_data GetLocalData() const;

	// parent
private:
	ListView& GetParent();

	// child
private:
	ref_ptr<TextView> text_view;
	ref_ptr<ListView> list_view;
private:
	TextView& GetTextView() { return *text_view; }
	ListView& GetListView() { return *list_view; }

	// layout
private:
	bool HitTestTextView(Point point) { return point.y < length_first; }
	Point ConvertToTextViewPoint(Point point);
	Point ConvertToListViewPoint(Point point);

	// paint
private:
	static constexpr Color color_unsaved = Color(Color::Green, 63);
	static constexpr Color color_error = Color(Color::Red, 63);
private:
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
	TextView& InsertAfterSelfOrFront(std::wstring text, bool ctrl);  // text split
	TextView& IndentSelf();  // text indent
	TextView& IndentSelfShift();  // text indent shift
	TextView& MergeBeforeSelf();  // text backspace
	TextView& MergeAfterSelf();  // text delete
	TextView& InsertAfterSelf(list_data text_list);  // text paste
public:
	ListView& InsertBack(std::vector<std::unique_ptr<PairView>> pair_view_list);  // list indent
	ListView& InsertAfterSelf(std::vector<std::unique_ptr<PairView>> pair_view_list);  // list indent shift
};
