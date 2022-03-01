#pragma once

#include "block_view.h"


BEGIN_NAMESPACE(WndDesign)

class BlockPairView;


class BlockListView : public BlockView, public LayoutType<Assigned, Auto> {
public:
	BlockListView(BlockView& parent) : BlockView(parent) {}

	// parent
private:
	BlockPairView& GetParent();

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
	void SetChildIndex(WndObject& child, size_t index) { WndObject::SetChildData<size_t>(child, index); }
	size_t GetChildIndex(WndObject& child) const { return WndObject::GetChildData<size_t>(child); }
	void UpdateIndex(size_t begin);
private:
	BlockPairView& GetChild(child_ptr& child);
	BlockPairView& GetChild(size_t index) { return GetChild(child_list[index].child); }
private:
	void InsertChild(size_t index, child_ptr child);
	void InsertChild(size_t index, std::vector<child_ptr> children);
	void EraseChild(size_t begin, size_t count);
	void AppendChild(child_ptr child) { InsertChild(-1, std::move(child)); }

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
	void UpdateSelectionRegion(size_t begin, size_t end);
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
	virtual void FinishDragDrop(BlockView& source) override;

	// input
private:
	void InsertAt(size_t index, std::wstring text);
	void InsertAt(size_t index, std::vector<std::wstring> text, size_t caret_pos);
	void InsertAt(size_t index, std::vector<std::unique_ptr<BlockPairView>> pair_view_list);
	void MergeBefore(size_t index);
	void MergeAfter(size_t index);
	void MergeAt(size_t index, BlockListView& list_view);
	std::unique_ptr<BlockPairView> Extract(size_t index);
	std::vector<std::unique_ptr<BlockPairView>> Extract(size_t begin, size_t end);
public:
	void InsertFront(std::wstring text) { InsertAt(0, text); }
	void InsertFront(std::vector<std::wstring> text, size_t caret_pos) { InsertAt(0, text, caret_pos); }
	void InsertBack(std::vector<std::unique_ptr<BlockPairView>> pair_view_list) { InsertAt(child_list.size(), std::move(pair_view_list)); }
	void InsertAfter(BlockView& child, std::wstring text) { InsertAt(GetChildIndex(child) + 1, text); }
	void InsertAfter(BlockView& child, std::vector<std::wstring> text, size_t caret_pos) { InsertAt(GetChildIndex(child) + 1, text, caret_pos); }
	void InsertAfter(BlockView& child, std::vector<std::unique_ptr<BlockPairView>> pair_view_list) { InsertAt(GetChildIndex(child) + 1, std::move(pair_view_list)); }
	void MergeBefore(BlockView& child) { MergeBefore(GetChildIndex(child)); }
	void MergeAfter(BlockView& child) { MergeAfter(GetChildIndex(child)); }
	void MergeFrontWith(BlockListView& list_view) { return MergeAt(0, list_view); }
	void MergeBackWith(BlockListView& list_view) { return MergeAt(child_list.size(), list_view); }
	std::unique_ptr<BlockPairView> PopFront() { return child_list.empty() ? nullptr : Extract(0); }
private:
	void Delete();
	void Indent();

	// message
private:
	virtual void OnKeyMsg(KeyMsg msg) override;
};


END_NAMESPACE(WndDesign)