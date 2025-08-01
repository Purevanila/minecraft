#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D hotbarTexture;
uniform vec3 color;
uniform float alpha;

void main() {
    vec4 texColor = texture(hotbarTexture, TexCoord);
    
    // Discard fully transparent pixels
    if (texColor.a < 0.1) {
        discard;
    }
    
    FragColor = vec4(texColor.rgb * color, texColor.a * alpha);
}
