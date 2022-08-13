#pragma once

#include "block_view.h"


class PairView;


class ListView : public BlockView, public LayoutType<Assigned, Auto> {
public:
	ListView(RootFrame& root) : BlockView(root) {}
	ListView(BlockView& parent) : BlockView(parent) {}

	// context
public:
	BlockView::IsRoot;
private:
	PairView& GetParent();

	// data
private:
	virtual void Load() override;
	virtual void Save() override;

	// child
private:
	using child_ptr = child_ptr<Assigned, Auto>;
private:
	struct ChildInfo {
	public:
		child_ptr child;
		float offset = 0.0f;
		float length = 0.0f;
	public:
		ChildInfo(child_ptr child) : child(std::move(child)) {}
	public:
		float BeginOffset() const { return offset; }
		float EndOffset() const { return offset + length; }
	};
	std::vector<ChildInfo> child_list;
private:
	using child_iter = std::vector<ChildInfo>::iterator;
public:
	bool IsEmpty() const { return child_list.empty(); }
private:
	PairView& GetChild(child_ptr& child);
	PairView& GetChild(size_t index) { return GetChild(child_list[index].child); }
private:
	void SetChildIndex(WndObject& child, size_t index) { WndObject::SetChildData<size_t>(child, index); }
	size_t GetChildIndex(WndObject& child) const { return WndObject::GetChildData<size_t>(child); }
	void UpdateIndex(size_t begin);

	// layout
private:
	Size size;
private:
	Rect GetChildRegion(size_t index) const { return Rect(Point(0.0f, child_list[index].offset), Size(size.width, child_list[index].length)); }
	Rect GetChildRegion(WndObject& child) const { return GetChildRegion(GetChildIndex(child)); }
	child_iter HitTestItem(float offset);
private:
	void UpdateLayout(size_t index);
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

	// selection
private:
	size_t selection_begin = 0;
private:
	void RedrawSelectionRegion();
	void UpdateSelectionRegion(size_t begin, size_t length);
private:
	virtual bool HitTestSelection(Point point) override;
	virtual void BeginSelect(BlockView& child) override;
	virtual void DoSelect(Point point) override;
	virtual void SelectChild(BlockView& child) override;
	virtual void SelectMore() override;
	virtual void ClearSelection() override;

	// drag and drop
private:
	void RedrawDragDropCaretRegion();
	void UpdateDragDropCaretRegion(size_t pos);
public:
	void DoDragDropBefore(BlockView& child) { UpdateDragDropCaretRegion(GetChildIndex(child)); }
private:
	virtual void DoDragDrop(BlockView& source, Point point) override;
	virtual void CancelDragDrop() override;

	// modify
private:
	void InsertChild(size_t index, child_ptr child);
	void InsertChild(size_t index, std::vector<child_ptr> children);
	void EraseChild(size_t begin, size_t length);
private:
	PairView& InsertChild(size_t index, std::wstring text);
	PairView& InsertChild(size_t index, std::vector<std::wstring> text_list);
	PairView& InsertChild(size_t index, std::unique_ptr<PairView> pair_view);
	void InsertChild(size_t index, std::vector<std::unique_ptr<PairView>> pair_view_list);
	std::unique_ptr<PairView> ExtractChild(size_t index);
	std::vector<std::unique_ptr<PairView>> ExtractChild(size_t begin, size_t length);

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
	PairView& InsertAfter(PairView& child, std::vector<std::wstring> text_list);  // text paste

	// input
private:
	virtual void FinishDragDrop(BlockView& source) override;
private:
	void Delete();
	void Indent();

	// message
private:
	virtual void OnKeyMsg(KeyMsg msg) override;
};
