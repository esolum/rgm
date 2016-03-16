#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
in vec3 lightPosition;
out vec3 fragNor;
out vec3 lightDir;
out vec3 View;
out vec3 vColor;
out vec2 vTexCoord;


void main()
{
    gl_Position = P * V * M * vertPos;
    fragNor = (V * M * vec4(vertNor, 0.0)).xyz;
    vec3 position_worldspace = (V * M * vertPos).xyz;
    View = 0 - (V * M *vertPos).xyz;
    
    lightDir = lightPosition - position_worldspace;
    
    /* a color that could be blended - or be shading */
    vColor = vec3(max(dot(fragNor, normalize(lightDir)), 0));
    /* pass through the texture coordinates to be interpolated */
    vTexCoord = vertTex;
}