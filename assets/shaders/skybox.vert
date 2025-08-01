#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 TexCoords;

void main()
{
    TexCoords = aPos;
    
    // Remove translation from the view matrix
    mat4 viewNoTranslation = mat4(mat3(view));
    vec4 pos = projection * viewNoTranslation * vec4(aPos, 1.0);
    
    // Set z = w so that the depth is always 1.0 (far plane)
    gl_Position = pos.xyww;
}
