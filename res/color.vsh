//
// 色
//

attribute vec4 position;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform vec4 material_diffuse;
uniform vec4 material_emissive;

varying vec4 dstColor;


void main() {
	dstColor = material_diffuse + material_emissive;

	gl_Position = projectionMatrix * modelViewMatrix * position;
}
