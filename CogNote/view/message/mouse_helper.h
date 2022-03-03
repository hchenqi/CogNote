#pragma once

#include "WndDesign/message/mouse_msg.h"
#include "WndDesign/message/timer.h"


using namespace WndDesign;


enum class MouseHelperMsg {
	None,
	Down,
	Click,
	MultipleClick,
	Drag,
	Drop,
};


class MouseHelper {
private:
	static constexpr uint timer_interval = 500;  // 500ms
	Timer timer = Timer([&]() { has_mouse_clicked = false; timer.Stop(); });
public:
	bool is_mouse_down = false;
	bool is_dragging = false;
	bool has_mouse_clicked = false;
	Point mouse_down_position;
public:
	MouseHelperMsg Track(MouseMsg msg) {
		switch (msg.type) {
		case MouseMsg::LeftDown:
			mouse_down_position = msg.point;
			timer.Set(timer_interval);
			if (has_mouse_clicked) {
				is_mouse_down = false;
				return MouseHelperMsg::MultipleClick;
			} else {
				is_mouse_down = true;
				has_mouse_clicked = true;
				return MouseHelperMsg::Down;
			}
			break;
		case MouseMsg::LeftUp:
			if (is_mouse_down) {
				is_mouse_down = false;
				if (is_dragging) {
					is_dragging = false;
					return MouseHelperMsg::Drop;
				} else {
					return MouseHelperMsg::Click;
				}
			}
			break;
		case MouseMsg::Move:
			if (msg.point != mouse_down_position) {
				has_mouse_clicked = false;
			}
			if (is_mouse_down) {
				is_dragging = true;
				return MouseHelperMsg::Drag;
			}
			break;
		}
		return MouseHelperMsg::None;
	}
};
