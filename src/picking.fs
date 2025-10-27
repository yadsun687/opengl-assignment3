#version 330

uniform uint gObjectIndex;
uniform uint gDrawIndex;

in vec4 worldPosition;
out vec3 FragColor;

void main()
{
    FragColor = vec3(worldPosition.xyz);
}
