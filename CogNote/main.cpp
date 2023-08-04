#include "view/RootFrame.h"

#include "WndDesign/window/Global.h"
#include "WndDesign/widget/TitleBarFrame.h"
#include "WndDesign/control/Scrollbar.h"
#include "WndDesign/layout/SplitLayout.h"


struct MainFrameStyle : public TitleBarFrame::Style {
	MainFrameStyle() {
		width.min(200px).normal(800px).max(100pct);
		height.min(200px).normal(500px).max(100pct);
		position.setHorizontalCenter().setVerticalCenter();
		border.width(4px).radius(10px).color(Color::CadetBlue);
		title.assign(L"CogNote");
		title_format.font.size(16px);
		title_format.paragraph.line_spacing(120pct);
	}
};


class ScrollBox : public SplitLayoutHorizontal<Assigned, Assigned, Auto, Assigned> {
protected:
	using ScrollFrame = ScrollFrame<Vertical>;
	using Scrollbar = Scrollbar<Vertical>;

public:
	ScrollBox() : SplitLayoutHorizontal{ new Frame(), new Scrollbar() } {
		GetScrollbar().SetScrollFrame(GetScrollFrame());
	}

protected:
	class Frame : public RootFrame {
		ScrollBox& GetScrollbox() const { return static_cast<ScrollBox&>(GetParent()); }
		virtual void OnFrameOffsetUpdate() override { GetScrollbox().GetScrollbar().UpdateScrollOffset(); }
	};

protected:
	ScrollFrame& GetScrollFrame() const { return static_cast<ScrollFrame&>(*child_first); }
	Scrollbar& GetScrollbar() const { return static_cast<Scrollbar&>(*child_second); }
};


int main() {
	global.AddWnd(
		new TitleBarFrame{
			MainFrameStyle(),
			new SolidColorBackground<ScrollBox>
		}
	);
	global.MessageLoop();
	return 0;
}
