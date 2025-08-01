#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float time;

void main() {
    // Pure white clouds like Minecraft
    vec3 cloudColor = vec3(1.0, 1.0, 1.0);
    
    // Minecraft-style lighting with slight face shading
    float ambientStrength = 0.9;
    vec3 ambient = ambientStrength * lightColor;
    
    // Add subtle directional lighting to show the 3D cube faces
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0) * 0.15; // Subtle shading to show 3D shape
    vec3 diffuse = diff * lightColor;
    
    vec3 result = (ambient + diffuse) * cloudColor;
    
    // Create Minecraft-style cloud texture with holes
    vec2 cloudCoord = TexCoord * 8.0; // 8x8 texture pattern per face
    
    // Add slow movement
    float moveSpeed = 0.002;
    cloudCoord.x += time * moveSpeed;
    
    // Solid Minecraft-style clouds - no holes, no cheese!
    vec2 pixelGrid = floor(cloudCoord);
    float pixelHash = sin(pixelGrid.x * 127.1 + pixelGrid.y * 311.7) * 43758.5453;
    pixelHash = fract(pixelHash);
    
    // Completely solid clouds with consistent alpha
    float alpha = 0.6; // Medium alpha that looks good even when overlapping
    
    FragColor = vec4(result, alpha);
}
