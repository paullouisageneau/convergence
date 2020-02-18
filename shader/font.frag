#version 330
precision highp float;
precision highp sampler2D;

uniform sampler2D color;

in vec3 fragPosition;
in vec2 fragTexCoord;

out vec4 fragColor;

void main()
{
	fragColor = texture(color, fragTexCoord);
    if(fragColor.a < 0.01)
        discard;
}

