#version 110

uniform vec3 lightPosition;

varying vec3 fragPosition;
varying vec3 fragNormal;
varying vec3 fragLight;
varying vec4 fragMaterial;

void main()
{
	float light = clamp(dot(fragNormal, -fragLight), 0.0, 1.0);
	float z = gl_FragCoord.z/gl_FragCoord.w;
	float fog = (exp2(0.1*z) - 1.0)*0.02;
	vec3 ambient = vec3(0.1, 0.1, 0.1)*fragMaterial.x + vec3(0.1, 0.25, 0.1)*fragMaterial.y;
	vec3 diffuse = vec3(0.2, 0.2, 0.2)*fragMaterial.x + vec3(0.2, 0.5, 0.2)*fragMaterial.y;
	vec3 color = (ambient + diffuse * light)*(1.0-fog) + vec3(1.0, 1.0, 1.0)*fog;
	gl_FragColor = vec4(color, 1.0);
}
