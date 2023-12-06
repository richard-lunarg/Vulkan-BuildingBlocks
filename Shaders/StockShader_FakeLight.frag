#version 450
// Fake light... just shades geometry as if the light were coming
// from the viewer. It's just a simple default 3D effect. Requires
// geometry have surface normals, and a solid color.


layout(location = 0) in vec4 vColor;
layout(location = 1) in vec3 vNormal;


// Output color
layout(location = 0) out vec4 vFragColor;

void main(void) { 
     vec3 vLightDir = vec3(0.0, 0.0, 1.0); // Always from view direction

    // This lights the back as well as the front
    float fDot = abs(dot(vLightDir, vNormal));
    
    vFragColor = vec4(fDot, fDot, fDot, 1.0) * vColor;
    }
