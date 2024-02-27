#include "view/TabView.h"

#include "WndDesign/window/Global.h"
#include "WndDesign/widget/TitleBarFrame.h"

#include "BlockStore/block_manager.h"


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
	//block_manager.open_file("CogNote.db");

	//block_ref b = block_manager.get_root();
	//block<std::wstring> text; text.write(L"words");
	//block<std::vector<std::pair<block_ref, block_ref>>> root; root.write({ {text, b} });
	//block_manager.set_root(root);

	//return 0;

	global.AddWnd(
		new TitleBarFrame{
			MainFrameStyle(),
			new TabView()
		}
	);
	global.MessageLoop();
	return 0;
}
