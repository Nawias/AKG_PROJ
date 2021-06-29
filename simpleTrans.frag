#version 400
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
in vec4 MatColor;
  
uniform vec3 lightDirection; 
uniform vec3 viewPos; 
uniform vec3 lightColor;

void main()
{

    if(MatColor.w >= 0.9) discard;

    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec4 result = (vec4(ambient,1.0) + vec4(diffuse,1.0) + vec4(specular,1.0)) * MatColor;
    FragColor = result;
} 