//
// テクスチャ＋ライティングx3
//

attribute vec4 position;
attribute vec3 normal;
attribute vec2 uv;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat3 normalMatrix;

uniform vec3 lightPosition[3];
uniform vec3 diffuse[3];
uniform vec3 ambient[2];
uniform vec3 specular[2];

uniform vec4 material_diffuse;
uniform vec4 material_emissive;
uniform vec4 material_specular;
uniform float material_shininess;

varying vec4 dstColor;
varying vec4 dstShine;
varying vec2 uv_out;


void main() {
	vec3 N = normalize(normalMatrix * normal);

	vec3  L0  = normalize(lightPosition[0]);
	float df0 = max(0.0, dot(N, L0));
	float sf0 = pow(df0, material_shininess);

	vec3  L1  = normalize(lightPosition[1]);
	float df1 = 1.0 - max(0.0, dot(N, L1));
	float sf1 = pow(df1, material_shininess);

	vec3  L2  = normalize(lightPosition[2]);
	float df2 = max(0.0, dot(N, L2));

	dstColor = material_diffuse * vec4(ambient[0] + df0 * diffuse[0], 0.0)
           + material_diffuse * vec4(ambient[1] + df1 * diffuse[1], 0.0)
           + material_diffuse * vec4(df2 * diffuse[2], 0.0);

  dstShine = material_specular * vec4(specular[0] * sf0, 0.0)
           + material_specular * vec4(specular[1] * sf1, 0.0)
           + material_emissive;

	uv_out = uv;
	
	gl_Position = projectionMatrix * modelViewMatrix * position;
}
