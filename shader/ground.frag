#version 330
precision highp float;
precision highp sampler3D;
precision highp samplerCube;

uniform float nearPlane;
uniform float farPlane;

uniform int   lightsCount;
uniform vec3  lightsPositions[16];
uniform vec4  lightsColors[16];
uniform float lightsPowers[16];
uniform samplerCube lightsDepthCubeMaps[16];

uniform sampler3D detail;

in vec3 fragPosition;
in vec3 fragNormal;
in vec4 fragAmbient;
in vec4 fragDiffuse;
in float fragSmoothness;

out vec4 fragColor;

float map(float value, float min1, float max1, float min2, float max2) {
	return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

float lightDepthCubeMap(int i, vec3 v) {
	// sampler arrays must be indexed with a constant
	float d;
	switch (i) {
		case 0: d = texture(lightsDepthCubeMaps[0], v).r; break;
		case 1: d = texture(lightsDepthCubeMaps[1], v).r; break;
		case 2: d = texture(lightsDepthCubeMaps[2], v).r; break;
		case 3: d = texture(lightsDepthCubeMaps[3], v).r; break;
		default: d = 1.0; break;
	}
	return map(d, 0.0, 1.0, nearPlane, farPlane);
}

vec2 poissonDisk[8] = vec2[](
	vec2(-0.613392, 0.617481),
	vec2(0.170019, -0.040254),
	vec2(-0.299417, 0.791925),
	vec2(0.645680, 0.493210),
	vec2(-0.651784, 0.717887),
	vec2(0.421003, 0.027070),
	vec2(-0.817194, -0.271096),
	vec2(-0.705374, -0.668203)
);

float lightDepthCubeMapShadow(int i, vec3 l) {
	vec3 up = vec3(0.0, 0.0, 1.0);
	vec3 u = normalize(cross(l, up));
	vec3 v = normalize(cross(u, l));
	float d = length(l);
	float bias = 0.5;
	float result = 0.0;
	for(int k=0; k<8; ++k) {
		vec2 p = poissonDisk[k] / 100.0;
		if(d < lightDepthCubeMap(i, l + u*p.x + v*p.y) + bias)
			result+= 0.125f;
	}
	return result;
}

void main()
{
	vec3 normal = normalize(fragNormal);

	vec4 lightColor = vec4(0.0, 0.0, 0.0, 0.0);
	for(int i = 0; i < lightsCount; ++i) {
		vec3 fragToLight = fragPosition - lightsPositions[i];
		float light = dot(normal, -normalize(fragToLight));
		if(light <= 0.0) light = 0.0;
		else {
			float cutoff = min(exp2(-length(fragToLight) / lightsPowers[i]), 1.0);
			light = cutoff * (1.0 + light) * 0.5;
			if(light < 0.1) light = 0.0;
			else if(light < 0.3) light = 0.3;
			else if(light < 0.5) light = 0.5;
			else light = 1.0;

			light *= lightDepthCubeMapShadow(i, fragToLight);
		}
		lightColor = min(lightColor + lightsColors[i] * light, vec4(1.0));
	}

	float z = gl_FragCoord.z/gl_FragCoord.w;
	float fog = min((exp2(0.2 * z) - 1.0) * 0.02, 1.0);
	vec4 color = fragAmbient + fragDiffuse * lightColor;
	// fragPosition - position
	// texture(detail, texcoord).xyz
	fragColor = vec4(color.xyz * (1.0 - fog), color.w);
}

