#include "view/RootFrame.h"

#include "WndDesign/window/Global.h"
#include "WndDesign/widget/TitleBarFrame.h"


struct MainFrameStyle : public TitleBarFrame::Style {
	MainFrameStyle() {
		width.min(200px).normal(800px).max(100pct);
		height.min(200px).normal(500px).max(100pct);
		position.setHorizontalCenter().setVerticalCenter();
		border.width(4px).radius(10px).color(Color::CadetBlue);
		title.assign(L"CogNote");
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
