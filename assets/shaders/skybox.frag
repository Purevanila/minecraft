#version 330 core

in vec3 TexCoords;
out vec4 FragColor;

uniform float time;

void main()
{
    // Normalize the texture coordinates to get the direction vector
    vec3 direction = normalize(TexCoords);
    
    // Calculate the height factor (y component, ranges from -1 to 1)
    float height = direction.y;
    
    // Minecraft-style sky colors
    // Zenith (top): darker blue #4A90E2 (74, 144, 226)
    // Horizon: lighter blue #87CEEB (135, 206, 235)
    
    vec3 zenithColor = vec3(74.0/255.0, 144.0/255.0, 226.0/255.0);    // Dark blue at top
    vec3 horizonColor = vec3(135.0/255.0, 206.0/255.0, 235.0/255.0);  // Light blue at horizon
    
    // Create smooth transition from horizon to zenith
    // height ranges from -1 (bottom) to 1 (top)
    // We want smooth transition, so use smoothstep for better gradients
    float heightFactor = (height + 1.0) * 0.5; // Convert from [-1,1] to [0,1]
    heightFactor = smoothstep(0.0, 1.0, heightFactor); // Smooth the transition
    
    // Mix colors based on height
    vec3 skyColor = mix(horizonColor, zenithColor, heightFactor);
    
    // Add a subtle time-based variation for atmosphere (very subtle)
    float timeVariation = sin(time * 0.1) * 0.02 + 1.0;
    skyColor *= timeVariation;
    
    FragColor = vec4(skyColor, 1.0);
}
