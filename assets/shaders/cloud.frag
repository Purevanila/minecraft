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
    // Simple cloud color - white with some transparency
    vec3 cloudColor = vec3(1.0, 1.0, 1.0);
    
    // Very basic lighting for clouds
    float ambientStrength = 0.9;
    vec3 ambient = ambientStrength * lightColor;
    
    // Minimal directional lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0) * 0.1;
    vec3 diffuse = diff * lightColor;
    
    vec3 result = (ambient + diffuse) * cloudColor;
    
    // Make clouds semi-transparent like Minecraft
    float alpha = 0.8;
    
    // Minecraft-style cloud texture pattern - very smooth and continuous
    float animatedX = TexCoord.x + time * 0.0005; // Very slow for smooth movement
    float animatedY = TexCoord.y + time * 0.0003; // Even slower
    
    // Simple cloud density pattern like Minecraft - smooth and continuous
    float cloudDensity = sin(animatedX * 3.14159) * sin(animatedY * 3.14159) * 0.1 + 0.9;
    
    // Subtle variation for realism
    float subtleVariation = sin(animatedX * 2.0) * 0.03 + 1.0;
    
    alpha *= cloudDensity * subtleVariation;
    
    // Smooth edge transitions like Minecraft
    alpha = smoothstep(0.6, 1.0, alpha);
    
    FragColor = vec4(result, alpha);
}
