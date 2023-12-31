#version 450
// Fake light... just shades geometry as if the light were coming
// from the viewer. It's just a simple default 3D effect. Requires
// geometry have surface normals, and a solid color.
// This version adds texture support, simply multiplies the texel
// by the light/color value.


layout(push_constant) uniform PC {
    mat4 mvpMatrix;     // Modelview Projection matrix
    mat4 packed;        // Normal matrix and color packed       
} PushConstants;


// Atributes for the geometry
// We only care about vertex positions inbound
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;


// Interpolate towards fragment shader
layout(location = 0) out vec4 vColor;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vTexCoord;

void main(void) {   
    // Do the lighting stuff
    // Extract rotation matrix from packed matrix
    mat3 rot = mat3(normalize(PushConstants.packed[0].xyz),
                normalize(PushConstants.packed[1].xyz),
                normalize(PushConstants.packed[2].xyz));
                
	// Output to fragment shader
	vNormal = rot * normalize(inNormal);
    vColor = PushConstants.packed[3];

	// Normal geometry transformation stuff
    gl_Position = PushConstants.mvpMatrix * vec4(inPosition, 1.0); 
    }
