//
// テクスチャ+ライティングx3
//

uniform sampler2D sampler;

varying lowp vec4 dstColor;
varying lowp vec4 dstShine;
varying mediump vec2 uv_out;


void main() {
  gl_FragColor = dstColor - texture2D(sampler, uv_out) + dstShine;
}
