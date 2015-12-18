//
// ういろうの影
//

attribute vec4 position;
attribute vec2 uv;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

varying vec2 uv_out;


void main() {
	uv_out = uv;
	gl_Position = projectionMatrix * modelViewMatrix * position;
}
