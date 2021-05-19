#version 150

in vec3 vPosition;
uniform mat4 ModelView, Projection;
uniform vec3 Color;
uniform int Draw;

out vec4 color;

void main()
{
  if(Draw==1){
    color = vec4(Color, 1.0);
  }
  else{
    color = vec4(Color, 0.3);
  }
  vec4 v = vec4(vPosition.xyz, 1);

  gl_Position = Projection * ModelView * v;
}
