#version 450
// StockShader_Flat.vert
// Flat shader. Draws any kind of geometry, takes vertex positions, an mvp matrix uniform,
// and a single color uniform.


layout(binding = 0) uniform UBO {
   mat4 mvp;
   vec4 vColor;
} ubo;

// We only care about vertex positions inbound
layout(location = 0) in vec4 inPosition;

layout(location = 0) out vec4 outColor;


void main() {
  gl_Position = ubo.mvp * inPosition;
  outColor = ubo.vColor;
}