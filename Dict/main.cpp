#include "WndDesign/window/Global.h"
#include "WndDesign/widget/TitleBarFrame.h"

#include "BlockStore/block_manager.h"


using namespace WndDesign;
using namespace BlockStore;


struct MainFrameStyle : public TitleBarFrame::Style {
	MainFrameStyle() {
		width.min(200px).normal(800px).max(100pct);
		height.min(200px).normal(500px).max(100pct);
		position.setHorizontalCenter().setVerticalCenter();
		border.width(4px).radius(10px).color(Color::CadetBlue);
		title.assign(L"Dict");
	}
};


int main() {
	block_manager.open_file("dict.db");
	global.AddWnd(
		new TitleBarFrame{
			MainFrameStyle(),
			new Placeholder<Assigned, Assigned>()
		}
	);
	global.MessageLoop();
	return 0;
}
