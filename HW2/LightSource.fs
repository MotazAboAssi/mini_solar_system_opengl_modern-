#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D diffuseTexture;
uniform vec3 objectColor;

void main()
{
    vec4 texColor = texture(diffuseTexture, TexCoord);
    FragColor = texColor * vec4(objectColor, 1.0);
}
