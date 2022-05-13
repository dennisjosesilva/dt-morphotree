#version 400 core 
layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

in vec2 gpos[];

smooth out vec2 fpos;
smooth out vec2 texCoord;

uniform mat4 viewProjection;
uniform int splatRadius;

void main()
{
  vec2 centerPoint = gpos[0];

  vec2 tl = centerPoint + vec2(-splatRadius, splatRadius);
  vec2 tr = centerPoint + vec2(splatRadius, splatRadius);
  vec2 br = centerPoint + vec2(splatRadius, -splatRadius);
  vec2 bl = centerPoint + vec2(-splatRadius, -splatRadius);

  gl_Position = viewProjection * vec4(tl, 0.0, 1.0);
  fpos = tl;
  texCoord = vec2(0,0);
  EmitVertex();

  gl_Position = viewProjection * vec4(tr, 0.0, 1.0);
  fpos = tr;
  texCoord = vec2(1,0);
  EmitVertex();

  gl_Position = viewProjection * vec4(bl, 0.0, 1.0);
  fpos = bl;
  texCoord = vec2(0, 1);
  EmitVertex();
  EndPrimitive();

  gl_Position = viewProjection * vec4(bl, 0.0, 1.0);
  fpos = bl;
  texCoord = vec2(0,1);
  EmitVertex();

  gl_Position = viewProjection * vec4(br, 0.0, 1.0);
  fpos = br;
  texCoord = vec2(1,1);
  EmitVertex();

  gl_Position = viewProjection * vec4(tr, 0.0, 1.0);
  fpos = tr;
  texCoord = vec2(1,0);
  EmitVertex();
  EndPrimitive();
}