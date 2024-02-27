#pragma once

#include "block.h"

#include "WndDesign/frame/WndFrame.h"


using namespace WndDesign;


class TabList;
class TabItemView;


class TabView : public WndFrame, public LayoutType<Assigned, Assigned> {
public:
	TabView();
	~TabView();
private:
	ref_ptr<TabList> list;
	ref_ptr<TabItemView> view;
};
