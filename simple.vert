#version 400
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTex;
layout (location = 3) in vec4 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec4 MatColor;

uniform mat4 MatM[9];
uniform mat4 MatV;
uniform mat4 MatP;
uniform mat4 MatWorld;
void main()
{
mat4 model = MatM[gl_InstanceID];

    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    MatColor = aColor;
    mat4 mvp = MatP * MatV * MatWorld;
    gl_Position = mvp * vec4(FragPos, 1.0);
}