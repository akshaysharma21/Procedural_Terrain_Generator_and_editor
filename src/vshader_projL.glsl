#version 150

in vec3 vPosition;
uniform mat4 ModelView, Projection;
uniform vec3 Color;
in vec4 vNormal;
uniform vec3 LightPos;
uniform mat4 ModelViewInverseTranspose;
uniform int Draw;


out vec4 color;

void main()
{
  vec3 lightDir = normalize(vec3(0.0, 0.0, -1.0));//normalize(LightPos - vPosition.xyz);
  vec3 norm = normalize( ModelViewInverseTranspose*vNormal ).xyz;
  // float cosAngle =  (vNormal.x * lightDir.x + vNormal.y * lightDir.y + vNormal.z * lightDir.z)/ (sqrt(pow(vNormal.x,2) + pow(vNormal.y,2) + pow(vNormal.z,2)) * sqrt(pow(lightDir.x,2) + pow(lightDir.y,2) + pow(lightDir.z,2)));
  float height =  vPosition.y;
  vec3 col;
  if(height<-0.4){
    col = vec3(0.8, 0.8, 0.8);
  }
  else if(height <0.1){
    col = Color;
  }
  else if (height<0.2){
    col = vec3(0.9, 0.7, 0.5);
  }
  else {
    col = vec3(0.1, 0.5, 0.9);
  }

  float prod = max( dot(lightDir, norm), 0.2 );
  if(Draw == 1){
    color = vec4(col * prod, 0.3);
  }
  else{
    color = vec4(col * prod, 1.0);
  }
  vec4 v = vec4(vPosition.xyz, 1);

  gl_Position = Projection * ModelView * v;
}
