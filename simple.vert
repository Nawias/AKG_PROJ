#version 400
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTex;
layout (location = 3) in vec4 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec4 MatColor;

uniform mat4 MatM;
uniform mat4 MatV;
uniform mat4 MatMVP;

void main()
{
    

    FragPos = vec3(MatM * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(MatM))) * aNormal;  
    MatColor = aColor;
    gl_Position = MatMVP * vec4(FragPos, 1.0);
}