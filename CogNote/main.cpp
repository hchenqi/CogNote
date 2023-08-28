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


int main() {
	global.AddWnd(
		new TitleBarFrame{
			MainFrameStyle(),
			new SolidColorBackground<RootFrame>
		}
	);
	global.MessageLoop();
	return 0;
}
