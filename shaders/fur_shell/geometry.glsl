#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

in vec2 TexCoord[];
in vec3 Normal[];
in mat3 TangentToWorld[];
in vec3 FragPos[];
in vec4 sshadowCoord[];
in vec3 vertColor[];
flat in int instanceId[];

out vec2 ShellTexCoord;
out vec3 ShellNormal;
out mat3 ShellTangentToWorld;
out vec3 ShellFragPos;
out vec4 ShellSshadowCoord;
out vec3 ShellVertColor;
flat out int ShellInstanceId;
out float shellHeight;

uniform mat4 projview;
uniform mat4 lightsprojview;
uniform mat4 model;
uniform float time;
uniform float _windStrength;

void emitShellVertex(int vertexIndex, float height) {
  vec3 normal = normalize(Normal[vertexIndex]);
  float modelScale = (length(model[0].xyz) + length(model[1].xyz) + length(model[2].xyz)) / 3.0;
  float gust = cos(time * 0.8 + FragPos[vertexIndex].x * 0.35 + FragPos[vertexIndex].z * 0.2);
  float sway = 0.3 * _windStrength * height * gust;
  vec3 windOffset = vec3(sway, 0.0, sway);
  vec3 position = FragPos[vertexIndex] + normal * (height * 0.08 * modelScale) + windOffset;

  ShellTexCoord = TexCoord[vertexIndex];
  ShellNormal = normal;
  ShellTangentToWorld = TangentToWorld[vertexIndex];
  ShellFragPos = position;
  ShellSshadowCoord = lightsprojview * vec4(position, 1.0);
  ShellVertColor = vertColor[vertexIndex];
  ShellInstanceId = instanceId[vertexIndex];
  shellHeight = height;
  gl_Position = projview * vec4(position, 1.0);
  EmitVertex();
}

void main() {
  for (int shell = 0; shell <= 5; shell++) {
    float height = float(shell) / 5.0;
    for (int vertex = 0; vertex < 3; vertex++) {
      emitShellVertex(vertex, height);
    }
    EndPrimitive();
  }
}
