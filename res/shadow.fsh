//
// ういろうの影
//

uniform lowp vec4 material_diffuse;
uniform sampler2D sampler;

varying mediump vec2 uv_out;


void main() {
  gl_FragColor = texture2D(sampler, uv_out) * material_diffuse;
}
