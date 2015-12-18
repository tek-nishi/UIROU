//
// 背景
//

attribute vec4 position;
attribute vec4 vtx_color;

uniform vec4 diffuse;

varying vec4 dst_color;


void main() {
  // 頂点カラー
  dst_color = vtx_color + diffuse;

  // デバイス座標系[-1, 1]の頂点をそのまま使う
  gl_Position = position;
}
