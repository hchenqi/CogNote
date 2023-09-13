#pragma once

#include "block.h"
#include "local_data.h"

#include "WndDesign/layout/ListLayout.h"
#include "WndDesign/figure/shape.h"


class PairView;


class ListView : public BlockView<std::vector<block_ref>>, public ListLayout<Vertical> {
private:
	using Base = ListLayout;

public:
	ListView(RootFrame& root) : BlockView(root), Base(gap) {}
	ListView(Block& parent) : BlockView(parent), Base(gap) {}
	ListView(Block& parent, list_data data) : ListView(parent) { if (!data.empty()) { InsertChild(0, data); } }

	// context
public:
	Block::IsRoot;
private:
	PairView& GetParent();

	// data
private:
	virtual void Set(const value_type& value) override;
	virtual value_type Get() override;
public:
	list_data GetLocalData(size_t begin, size_t length) const;
	list_data GetLocalData() const { return GetLocalData(0, Length()); }

	// style
private:
	static constexpr float gap = 1.0f;

	// child
private:
	PairView& GetChild(WndObject& child) const;
	PairView& GetChild(size_t index) const { return GetChild(Base::GetChild(index)); }

	// modify
private:
	PairView& InsertChild(size_t index, std::wstring text);
	PairView& InsertChild(size_t index, list_data text_list);
	PairView& InsertChild(size_t index, std::unique_ptr<PairView> pair_view);
	void InsertChild(size_t index, std::vector<std::unique_ptr<PairView>> pair_view_list);
	std::unique_ptr<PairView> ExtractChild(size_t index);
	std::vector<std::unique_ptr<PairView>> ExtractChild(size_t begin, size_t length);

	// paint
private:
	virtual void OnDraw(FigureQueue& figure_queue, Rect draw_region) override;

	// caret
private:
	virtual void SetCaret(Point point) override;

	// selection
private:
	static constexpr Color selection_color = Color(Color::DimGray, 0x7f);
private:
	size_t selection_begin = 0;
	size_t selection_range_begin = 0;
	size_t selection_range_length = 0;
	Rect selection_region = region_empty;
private:
	size_t get_selection_range_end() { return selection_range_begin + selection_range_length; }
	bool PositionInSelection(size_t pos) { return selection_range_begin <= pos && pos < get_selection_range_end(); }
	bool PositionCoveredBySelection(size_t pos) { return selection_range_begin <= pos && pos <= get_selection_range_end(); }
private:
	void RedrawSelectionRegion();
	void UpdateSelectionRegion(size_t begin, size_t length);
private:
	virtual bool HitTestSelection(Point point) override;
	virtual void BeginSelect(Block& child) override;
	virtual void DoSelect(Point point) override;
	virtual void SelectChild(Block& child) override;
	virtual void SelectMore() override;
	virtual void ClearSelection() override;

	// drag and drop
private:
	static constexpr float drag_drop_caret_height = 1.0f;
	static constexpr Color drag_drop_caret_color = Color::DimGray;
private:
	size_t drag_drop_caret_position = 0;
	Rect drag_drop_caret_region = region_empty;
private:
	void RedrawDragDropCaretRegion();
	void UpdateDragDropCaretRegion(size_t pos);
public:
	void DoDragDropBefore(Block& child) { UpdateDragDropCaretRegion(GetChildIndex(dynamic_cast<WndObject&>(child))); }
private:
	virtual void DoDragDrop(Block& source, Point point) override;
	virtual void CancelDragDrop() override;
	virtual void FinishDragDrop(Block& source) override;

	// route
public:
	PairView& InsertBack(std::unique_ptr<PairView> pair_view);  // text indent
	PairView& InsertAfter(PairView& child, std::unique_ptr<PairView> pair_view);  // text indent shift
	ListView& MergeFrontWith(ListView& list_view);  // text backspace
	PairView& IndentAfterChild(PairView& child);  // text delete
	ListView& InsertBack(std::vector<std::unique_ptr<PairView>> pair_view_list);  // list indent
	ListView& InsertAfter(PairView& child, std::vector<std::unique_ptr<PairView>> pair_view_list);  // list indent shift
public:
	PairView& InsertAfter(PairView& child, std::wstring text);  // text split
	PairView& InsertFront(std::wstring text);  // text split
	PairView& IndentChild(PairView& child);  // text indent
	PairView& IndentChildShift(PairView& child);  // text indent shift
	PairView& MergeBeforeChild(PairView& child);  // text backspace
	PairView& MergeFrontChild();  // text delete
	PairView& MergeAfterChild(PairView& child);  // text delete
	PairView& InsertAfter(PairView& child, list_data text_list);  // text paste

	// input
private:
	void Delete();
	void Indent();

	// clipboard
protected:
	void Cut();
	void Copy();

	// message
private:
	virtual void OnKeyMsg(KeyMsg msg) override;
};
