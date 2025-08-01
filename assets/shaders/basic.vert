#version 330 core

// Input attributes from the mesh
layout (location = 0) in vec3 aPos;      // Vertex position
layout (location = 1) in vec2 aTexCoord; // Texture coordinates  
layout (location = 2) in vec3 aNormal;   // Surface normal

// Transformation matrices
uniform mat4 model;      // Object-to-world transformation
uniform mat4 view;       // World-to-camera transformation  
uniform mat4 projection; // Camera-to-screen transformation

// Output to fragment shader
out vec2 TexCoord;  // Pass through texture coordinates
out vec3 Normal;    // Transformed normal for lighting
out vec3 FragPos;   // World position for lighting calculations

void main() {
    // Transform vertex to screen space
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // Pass texture coordinates directly to fragment shader
    TexCoord = aTexCoord;
    
    // Transform normal to world space for lighting calculations
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Calculate world position for lighting
    FragPos = vec3(model * vec4(aPos, 1.0));
}
