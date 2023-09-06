#pragma once

#include "block.h"
#include "local_data.h"

#include "WndDesign/layout/SplitLayout.h"
#include "WndDesign/figure/shape.h"


class TextView;
class ListView;


class PairView : public Block, public SplitLayoutHorizontal<Auto, Assigned, Auto, Auto> {
private:
	using Base = SplitLayoutHorizontal;

public:
	PairView(Block& parent, pair_data data);

	// context
private:
	ListView& GetParent();

	// data
private:
	virtual void Load() override;
	virtual void Save() override;
public:
	pair_data GetLocalData() const;

	// child
private:
	ref_ptr<TextView> text_view;
	ref_ptr<ListView> list_view;
private:
	TextView& GetTextView() { return *text_view; }
	ListView& GetListView() { return *list_view; }

	// layout
private:
	bool HitTestTextView(Point point) { return point.x < length_first; }
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
	virtual void BeginSelect(Block& child) override;
	virtual void DoSelect(Point point) override;
	virtual void SelectChild(Block& child) override;

	// drag and drop
private:
	virtual void DoDragDrop(Block& source, Point point) override;

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
