
#pragma once

//
// メニュー向け雑用
//

#include "nn_textWidget.hpp"
#include "nn_touchWidget.hpp"


namespace ngs {
namespace menu {

// TextWidget を TouchWidget に追加する
TouchWidget::Handle addWidget(TouchWidget& widget, TextWidget& text, TouchWidget::CallBack call_back, const bool active) {
  TouchWidget::Handle handle = widget.addWidget(call_back, text.dispPos(), text.size(), text.viewLayout());
  widget.activeWidget(handle, active);

  return handle;
}

// 2D座標→3D座標
Vec3f layoutPosToWorld(const Vec3f& pos, const View::Layout layout, const View& view, const Mat4f& project, const Mat4f& model) {
  Vec2f layout_pos = view.layoutPos(Vec2f(pos.x(), pos.y()), layout);
  Vec2f win_pos    = view.toWindowPos(layout_pos);

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  Vec3f world_pos;
  pointUnProject(world_pos, Vec3f(win_pos.x(), win_pos.y(), pos.z()), model, project, viewport);

  return world_pos;
}


}
}
