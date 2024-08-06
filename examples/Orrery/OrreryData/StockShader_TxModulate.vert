#version 450
// StockShader_Flat.vert
// Flat shader. Draws any kind of geometry, takes vertex positions, an mvp matrix uniform,
// and a single color uniform.


layout(push_constant) uniform PC {
    mat4 mvpMatrix;     // Modelview Projection matrix
    mat4 packed;        // Normal matrix and color packed       
} PushConstants;


// We only care about vertex positions inbound
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 vTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outTexCoord;


void main() {
  gl_Position = PushConstants.mvpMatrix * vec4(inPosition, 1.0);
  outColor =  PushConstants.packed[3];
  outTexCoord = vTexCoord;
}