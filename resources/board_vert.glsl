#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform vec3 pointLight1 = vec3(6, 1.5, 3);
uniform vec3 pointLight2 = vec3(6, 1.5, -3);
uniform vec3 pointLight3 = vec3(-6, 1.5, 3);
uniform vec3 pointLight4 = vec3(-6, 1.5, -3);
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
out vec3 fragNor;
out vec3 View;
out vec2 vTexCoord;
out vec3 position;
out vec3 lightDir1;
out vec3 lightDir2;
out vec3 lightDir3;
out vec3 lightDir4;
out float att1;
out float att2;
out float att3;
out float att4;


void main()
{
    gl_Position = P * V * M * vertPos;
    fragNor = (V * M * vec4(vertNor, 0.0)).xyz;
    position = (V * M *  vertPos).xyz;
    View = 0 - (V * M *vertPos).xyz;
    
    lightDir1 = pointLight1 - position;
    lightDir2 = pointLight2 - position;
    lightDir3 = pointLight3 - position;
    lightDir4 = pointLight4 - position;
    
    float att = 5 / distance(position, pointLight1);
    att1 = min(att , 1.0);
    
    att = 5 / distance(position, pointLight2);
    att2 = min(att , 1.0);
    
    att = 5 / distance(position, pointLight3);
    att3 = min(att , 1.0);
    
    att = 5 / distance(position, pointLight4);
    att4 = min(att , 1.0);
    
    /* pass through the texture coordinates to be interpolated */
    vTexCoord = vertTex;
    
}