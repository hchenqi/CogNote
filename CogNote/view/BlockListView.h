#pragma once

#include "BlockPairView.h"


class BlockListView : public BlockView, public LayoutType<Assigned, Auto> {
public:
	BlockListView(RootFrame& root) : BlockView(root) {}
	BlockListView(BlockView& parent) : BlockView(parent) {}

	// context
public:
	BlockView::IsRoot;
private:
	BlockPairView& GetParent();

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
private:
	BlockPairView& GetChild(child_ptr& child);
	BlockPairView& GetChild(size_t index) { return GetChild(child_list[index].child); }
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
	virtual void SelectMore();
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
	BlockPairView& InsertChild(size_t index, std::wstring text);
	BlockPairView& InsertChild(size_t index, std::vector<std::wstring> text_list);
	BlockPairView& InsertChild(size_t index, std::unique_ptr<BlockPairView> pair_view);
	void InsertChild(size_t index, std::vector<std::unique_ptr<BlockPairView>> pair_view_list);
	std::unique_ptr<BlockPairView> ExtractChild(size_t index);
	std::vector<std::unique_ptr<BlockPairView>> ExtractChild(size_t begin, size_t length);

	// route
private:
	BlockPairView& Indent(size_t index);
	BlockPairView& MergeBefore(size_t index);
	BlockPairView& MergeAfter(size_t index);
	void MergeAtWith(size_t index, BlockListView& list_view);
public:
	BlockPairView& Indent(BlockView& child) { return Indent(GetChildIndex(child)); }
	BlockPairView& InsertFront(std::wstring text) { return InsertChild(0, text); }
	BlockPairView& InsertFront(std::vector<std::wstring> text_list) { return InsertChild(0, text_list); }
	BlockPairView& InsertBack(std::unique_ptr<BlockPairView> pair_view) { return InsertChild(child_list.size(), std::move(pair_view)); }
	BlockListView& InsertBack(std::vector<std::unique_ptr<BlockPairView>> pair_view_list) { InsertChild(child_list.size(), std::move(pair_view_list)); return *this; }
	BlockPairView& InsertAfter(BlockView& child, std::wstring text) { return InsertChild(GetChildIndex(child) + 1, text); }
	BlockPairView& InsertAfter(BlockView& child, std::vector<std::wstring> text_list) { return InsertChild(GetChildIndex(child) + 1, text_list); }
	BlockPairView& InsertAfter(BlockView& child, std::unique_ptr<BlockPairView> pair_view) { return InsertChild(GetChildIndex(child) + 1, std::move(pair_view)); }
	BlockListView& InsertAfter(BlockView& child, std::vector<std::unique_ptr<BlockPairView>> pair_view_list) { InsertChild(GetChildIndex(child) + 1, std::move(pair_view_list)); return *this; }
	BlockPairView& MergeBefore(BlockView& child) { return MergeBefore(GetChildIndex(child)); }
	BlockPairView& MergeAfter(BlockView& child) { return MergeAfter(GetChildIndex(child)); }
	void MergeFrontWith(BlockListView& list_view) { MergeAtWith(0, list_view); }
	void MergeBackWith(BlockListView& list_view) { MergeAtWith(child_list.size(), list_view); }
	std::unique_ptr<BlockPairView> PopFront() { return child_list.empty() ? nullptr : ExtractChild(0); }

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
