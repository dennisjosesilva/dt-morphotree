#version 400 core
out vec4 fcolor;

smooth in vec2 fpos;
smooth in vec2 texCoord;

uniform sampler2D splatTexture;

void main()
{
  float dst = texture(splatTexture, texCoord).r;
  fcolor = vec4(dst, dst, dst, 1);
}