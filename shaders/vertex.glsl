#version 400 core
layout (location = 0) in vec2 vpos;

out vec2 gpos;

void main()
{
  gpos = vpos;
}