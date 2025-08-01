#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 projection;
uniform vec2 position;
uniform vec2 size;

void main() {
    // Scale and position the quad
    vec2 scaledPos = aPos * size + position;
    
    gl_Position = projection * vec4(scaledPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
