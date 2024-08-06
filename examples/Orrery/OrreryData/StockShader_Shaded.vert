#version 450
// StockShader_Shaded.vert
// Shaded shader. Draws any kind of geometry, takes vertex positions and vertex color.
// The mvp matrix is a push constant


layout(push_constant) uniform PC {
   mat4 mvp;
} PushConstants;

// We only care about vertex positions inbound
layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;


void main() {
    gl_Position = PushConstants.mvp * inPosition;
    outColor = inColor;
}
