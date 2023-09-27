#pragma once

#include "block.h"
#include "local_data.h"
#include "block_view.h"

#include "WndDesign/control/EditBox.h"
#include "WndDesign/figure/text_block.h"
#include "WndDesign/common/unicode_helper.h"


using namespace WndDesign;


class PairView;


class TextView : public Block<std::wstring>, public BlockView, public EditBox {
public:
	TextView(BlockView& parent, std::wstring text);

	// data
private:
	virtual void Set(const value_type& value) override;
	virtual value_type Get() override;
public:
	text_data GetLocalData() const { return GetText(); }

	// parent
private:
	PairView& GetParent();

	// modify
private:
	virtual void OnTextUpdate() override { EditBox::OnTextUpdate(); DataModified(); }

	// paint
private:
	virtual void OnDraw(FigureQueue& figure_queue, Rect draw_region) override;

	// caret
public:
	void SetCaret(size_t position) { EditBox::SetCaret(position); SetCaretFocus(); }
private:
	virtual void SetCaret(Point point) override { EditBox::SetCaret(point); SetCaretFocus(); }
	virtual void ClearCaret() override { EditBox::HideCaret(); }

	// selection
private:
	virtual bool HitTestSelection(Point point) override;
	virtual void BeginSelect(BlockView& child) override {}
	virtual void DoSelect(Point point) override { EditBox::DoSelect(point); SetSelectionFocus(); }
	virtual void SelectMore() override;
	virtual void ClearSelection() override { EditBox::ClearSelection(); }

	// drag and drop
private:
	static constexpr float drag_drop_caret_width = 1.0f;
	static constexpr Color drag_drop_caret_color = Color::DimGray;
private:
	size_t drag_drop_caret_position = 0;
	Rect drag_drop_caret_region = region_empty;
private:
	void RedrawDragDropCaretRegion();
private:
	virtual void DoDragDrop(BlockView& source, Point point) override;
	virtual void CancelDragDrop() override;
	virtual void FinishDragDrop(BlockView& source) override;

	// route
public:
	TextView& MergeBackWith(TextView& text_view);  // text backspace, text delete

	// input
private:
	void Split();
	void Indent();
	void Delete(bool is_backspace);
private:
	void Paste();

	// message
private:
	virtual void OnKeyMsg(KeyMsg msg) override;
};
