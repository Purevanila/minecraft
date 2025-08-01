#version 330 core

// Input from the vertex shader
in vec2 TexCoord;  // Texture coordinates for sampling textures
in vec3 Normal;    // Surface normal (we'll use this for basic lighting)
in vec3 FragPos;   // Fragment position (not needed for simple lighting)

// Output color for this pixel
out vec4 FragColor;

// The texture we're applying to surfaces
uniform sampler2D texture1;

// Light uniforms (we'll use these for simple, consistent lighting)
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main() {
    // Sample the texture
    vec4 textureColor = texture(texture1, TexCoord);
    
    // Simple Minecraft-style lighting: bright and consistent
    // We'll use a high ambient light so everything is well-lit
    float ambientStrength = 0.8;  // High ambient = bright like Minecraft
    vec3 ambient = ambientStrength * lightColor;
    
    // Add just a tiny bit of directional lighting for some depth
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0) * 0.2;  // Very subtle directional light
    vec3 diffuse = diff * lightColor;
    
    // Combine for bright, even lighting like classic Minecraft
    vec3 result = (ambient + diffuse) * textureColor.rgb;
    
    // Use full opacity for all blocks (solid rendering)
    FragColor = vec4(result, textureColor.a);
}
