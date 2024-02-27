#include "TabView.h"
#include "RootFrame.h"

#include "WndDesign/control/Placeholder.h"
#include "WndDesign/control/EditBox.h"
#include "WndDesign/layout/SplitLayout.h"
#include "WndDesign/layout/ListLayout.h"
#include "WndDesign/frame/ScrollFrame.h"
#include "WndDesign/frame/PaddingFrame.h"
#include "WndDesign/frame/FixedFrame.h"
#include "WndDesign/wrapper/Background.h"

#include "BlockStore/block_manager.h"


using namespace BlockStore;


class TabItem : public Block<std::wstring>, public SolidColorBackground<WndFrame, color_transparent>, public LayoutType<Assigned, Auto> {
public:
	TabItem(block_ref text_block, block_ref view_block) : Base{
		new PaddingFrame {
			Padding(10px, 5px),
			text_box = new TextBox(TextBox::Style(), L"")
		}
	}, view_block(view_block) {
		LoadBlock(*this, text_block);
	}

private:
	ref_ptr<TextBox> text_box;

public:
	virtual void Set(const value_type& value) override { text_box->Assign(value.empty() ? L"New Item" : value); }
	virtual value_type Get() override { return text_box->GetText(); }
public:
	void SetText(const std::wstring& text) {
		DataModified();
		text_box->Assign(text);
	}

public:
	block_ref view_block;
public:
	ref_ptr<RootFrame> root_frame = nullptr;
public:
	std::pair<block_ref, block_ref> GetRef() {
		if (root_frame) { root_frame->Save(); }
		return { GetBlockRef(*this), view_block };
	}

private:
	virtual ref_ptr<WndObject> HitTest(MouseMsg& msg) override { return this; }
	virtual void OnMouseMsg(MouseMsg msg) override;
	virtual void OnNotifyMsg(NotifyMsg msg) override {
		switch (msg) {
		case NotifyMsg::MouseEnter: background = Color::LightYellow; Redraw(region_infinite); break;
		case NotifyMsg::MouseLeave: background = color_transparent; Redraw(region_infinite); break;
		}
	}
};


class Title : public PaddingFrame<Assigned, Auto> {
public:
	Title(TabItem& item) : PaddingFrame{
		Padding(0px, 5px),
		child_ptr() = new Editor(item)
	} {
	}

private:
	virtual void OnDraw(FigureQueue& figure_queue, Rect draw_region) override {
		PaddingFrame::OnDraw(figure_queue, draw_region);
		figure_queue.add(Point(0px, size.height - 5.0f), new Rectangle(Size(std::max(200.0f, size.width), 1px), Color::LightGray));
	}

private:
	class Editor : public EditBox {
	private:
		struct Style : public EditBox::Style {
			Style() {
				font.size(20px).weight(FontWeight::Bold);
			}
		};
	public:
		Editor(TabItem& item) : EditBox(Style(), item.Get()), item(item) {}
	private:
		TabItem& item;
	private:
		virtual void OnTextUpdate() override {
			EditBox::OnTextUpdate();
			item.SetText(text);
		}
	};
};


class TabItemView : public WndFrameMutable, public LayoutType<Assigned, Assigned> {
public:
	TabItemView() : WndFrameMutable(new Placeholder<Assigned, Assigned>()) {}

private:
	ref_ptr<TabItem> current_item = nullptr;
public:
	void ResetItem() {
		if (current_item) {
			current_item->root_frame = nullptr;
		}
		current_item = nullptr;
		Reset(new Placeholder<Assigned, Assigned>());
	}
	void SetItem(TabItem& item) {
		if (current_item == &item) { return; }
		if (current_item) {
			current_item->root_frame = nullptr;
		}
		current_item = nullptr;
		current_item = &item;
		Reset(
			new PaddingFrame{
				Padding(10px),
				new SplitLayoutVertical{
					new Title(item),
					item.root_frame = new RootFrame(item.view_block)
				}
			}
		);
	}
};


class TabList : public Block<std::vector<std::pair<block_ref, block_ref>>>, public ListLayout<Vertical> {
public:
	TabList() : ListLayout(1.0f) {}

public:
	ref_ptr<TabItemView> view = nullptr;

private:
	TabItem& GetChild(size_t index) { return static_cast<TabItem&>(ListLayout::GetChild(index)); }

private:
	virtual void Set(const value_type& value) override {
		std::vector<TabList::child_ptr> ptr_list;
		for (auto& pair : value) {
			ptr_list.emplace_back(new TabItem(pair.first, pair.second));
		}
		InsertChild(-1, std::move(ptr_list));
	}
	virtual value_type Get() override {
		value_type value;
		for (size_t index = 0; index < Length(); ++index) {
			value.push_back(GetChild(index).GetRef());
		}
		return value;
	}

public:
	void SetItem(TabItem& item) {
		view->SetItem(item);
	}
	void RemoveItem(TabItem& item) {
		view->ResetItem();
		EraseChild(GetChildIndex(item));
		DataModified();
	}

private:
	virtual void OnMouseMsg(MouseMsg msg) override {
		if (msg.type == MouseMsg::LeftDown) {
			AppendChild(new TabItem(block_ref(), block_ref()));
			DataModified();
		}
	}
};


TabView::TabView() : WndFrame{
	child_ptr<Assigned, Assigned>(new SplitLayoutHorizontal{
		new FixedFrame<Auto, Assigned>{
			200px,
			new ScrollFrame<Vertical>{
				list = new SolidColorBackground<TabList, Color::LightBlue>()
			}
		},
		view = new TabItemView()
	})
} {
	list->view = view;
	block_manager.open_file("CogNote.db");
	Block<>::LoadBlock(*list, block_manager.get_root());
}

TabView::~TabView() {
	Block<>::SaveAll();
	block_manager.set_root(Block<>::GetBlockRef(*list));
	block_manager.collect_garbage();
	block_manager.close_file();
}

inline void TabItem::OnMouseMsg(MouseMsg msg) {
	if (msg.type == MouseMsg::LeftDown) {
		static_cast<TabList&>(GetParent()).SetItem(*this);
	}
	if (msg.type == MouseMsg::RightDown && root_frame) {
		static_cast<TabList&>(GetParent()).RemoveItem(*this);
	}
}
