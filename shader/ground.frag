#version 330
precision highp float;
precision highp sampler3D;

uniform int   lightsCount;
uniform vec3  lightsPositions[16];
uniform vec4  lightsColors[16];
uniform float lightsPowers[16];

uniform sampler3D detail;

in vec3 fragPosition;
in vec3 fragNormal;
in vec4 fragAmbient;
in vec4 fragDiffuse;
in float fragSmoothness;

out vec4 fragColor;

void main()
{
	vec4 lightColor = vec4(0.0, 0.0, 0.0, 0.0);
	for(int i = 0; i < lightsCount; ++i) {
		vec3 v = fragPosition - lightsPositions[i];
		float l = clamp(dot(fragNormal, -v), 0.0, 1.0);
		l *= min(exp2(-length(v) / lightsPowers[i]), 1.0);
		if(l < 0.1) l = 0.0;
		else if(l < 0.4) l = 0.4;
		else if(l < 0.7) l = 0.7;
		else l = 1.0;
		lightColor = min(lightColor + lightsColors[i] * l, vec4(1.0));
	}

	float z = gl_FragCoord.z/gl_FragCoord.w;
	float fog = min((exp2(0.2 * z) - 1.0) * 0.02, 1.0);
	vec4 color = fragAmbient + fragDiffuse * lightColor;
	// fragPosition - position
	// texture(detail, texcoord).xyz
	fragColor = vec4(color.xyz * (1.0 - fog), color.w);
}

