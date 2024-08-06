#version 450
// StockShader_Flat.frag


layout(location = 0) in vec4 inColor;


layout(location = 0) out vec4 vFragColor;

void main() {
    vFragColor = inColor;
  }
