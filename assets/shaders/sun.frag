#version 330 core

in vec2 TexCoord;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D sunTexture;
uniform float time;
uniform float sunIntensity;

void main() {
    // Sample the sun texture
    vec4 texColor = texture(sunTexture, TexCoord);
    
    // Apply sun intensity (brightness based on position)
    vec3 sunColor = texColor.rgb * sunIntensity;
    
    // Add a subtle glow effect around the sun
    vec2 center = vec2(0.5, 0.5);
    float dist = length(TexCoord - center);
    float glow = 1.0 - smoothstep(0.0, 0.5, dist);
    glow = pow(glow, 2.0) * 0.3; // Soft glow
    
    // Combine texture with glow
    sunColor += vec3(1.0, 0.9, 0.7) * glow * sunIntensity;
    
    // Add subtle pulsing effect
    float pulse = sin(time * 2.0) * 0.05 + 1.0;
    sunColor *= pulse;
    
    // Preserve original alpha for proper blending
    FragColor = vec4(sunColor, texColor.a * sunIntensity);
}
