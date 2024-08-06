#version 450
// StockShader_Flat.frag

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 vTexCoord;


layout(location = 0) out vec4 vFragColor;

void main() {
    vFragColor = inColor * texture(texSampler, vTexCoord);
  }
