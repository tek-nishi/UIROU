//
// テクスチャ+ライティング
//

attribute vec4 position;
attribute vec3 normal;
attribute vec2 uv;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;

uniform vec3 lightPosition;
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;

uniform vec4 material_diffuse;
uniform vec4 material_emissive;
uniform vec4 material_specular;
uniform float material_shininess;

varying vec4 dstColor;
varying vec4 dstShine;
varying vec2 uv_out;


void main() {
	vec3 N = normalize(normalMatrix * normal);
	vec3 L = normalize(lightPosition);

	float df = max(0.0, dot(N, L));
	float sf = pow(df, material_shininess);

	dstColor = material_diffuse * vec4(ambient + df * diffuse, 1.0);
	dstShine = material_specular * vec4(specular * sf, 0.0) + material_emissive;
	uv_out   = uv;
	
	gl_Position = projectionMatrix * modelViewMatrix * position;
}
