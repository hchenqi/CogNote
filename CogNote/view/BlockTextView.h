#pragma once

#include "block_view.h"

#include "WndDesign/figure/text_block.h"
#include "WndDesign/common/unicode_helper.h"


class BlockPairView;


class BlockTextView : public BlockView, public LayoutType<Assigned, Auto> {
public:
	BlockTextView(BlockView& parent, std::wstring text);

	// parent
private:
	BlockPairView& GetParent();

	// text
private:
	using HitTestInfo = TextBlock::HitTestInfo;
private:
	std::wstring text;
	TextBlock text_block;
	WordBreakIterator word_break_iterator;
private:
	size_t GetCharacterLength(size_t text_position) { return GetUTF16CharLength(text[text_position]); }
private:
	void TextUpdated();
private:
	void SetText(std::wstring str) { text.assign(std::move(str)); TextUpdated(); }
	void AppendText(std::wstring str) { text.append(str); TextUpdated(); }
	void InsertText(size_t pos, wchar ch) { text.insert(pos, 1, ch); TextUpdated(); }
	void InsertText(size_t pos, std::wstring str) { text.insert(pos, str); TextUpdated(); }
	void ReplaceText(size_t begin, size_t length, wchar ch) { text.replace(begin, length, 1, ch); TextUpdated(); }
	void ReplaceText(size_t begin, size_t length, std::wstring str) { text.replace(begin, length, str); TextUpdated(); }
	void DeleteText(size_t begin, size_t length) { text.erase(begin, length); TextUpdated(); }

	// layout
private:
	float width = 0.0f;
private:
	Size UpdateLayout();
private:
	virtual Size OnSizeRefUpdate(Size size_ref) override;

	// paint
private:
	virtual void OnDraw(FigureQueue& figure_queue, Rect draw_region) override;

	// caret
private:
	void RedrawCaretRegion();
private:
	void SetCaret(const HitTestInfo& info);
public:
	void SetCaret(size_t text_position);
private:
	virtual void SetCaret(Point point) override;
	virtual void ClearCaret() override;

	// selection
private:
	void RedrawSelectionRegion();
	void UpdateSelectionRegion(size_t begin, size_t end);
private:
	void SelectWord();
private:
	virtual bool HitTestSelection(Point point) override;
	virtual void BeginSelect(BlockView& child) override;
	virtual void DoSelect(Point point) override;
	virtual void SelectMore() override;
	virtual void ClearSelection() override;

	// drag and drop
private:
	void RedrawDragDropCaretRegion();
private:
	virtual void DoDragDrop(BlockView& source, Point point) override;
	virtual void CancelDragDrop() override;
	virtual void FinishDragDrop(BlockView& source) override;

	// input
private:
	void Insert(wchar ch);
	void Insert(std::wstring str);
	void Delete(bool is_backspace);
	void Indent();
private:
	void Split();
	void OnImeBegin();
	void OnImeString();
	void OnImeEnd();
public:
	void MergeBackWith(BlockTextView& text_view);

	// clipboard
private:
	void Cut();
	void Copy();
	void Paste();

	// message
private:
	virtual void OnKeyMsg(KeyMsg msg) override;
};
