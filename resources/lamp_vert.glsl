#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec3 vertTex;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
in vec3 lightPosition;
in float normalShowing;
out vec3 fragNor;
out vec3 LightDirection_cameraspace;
out float mode;
out vec3 View;
out vec3 vTexCoord;


void main()
{
    gl_Position = P * V * M * vertPos;
    fragNor = (V * M * vec4(vertNor, 0.0)).xyz;
    vec3 position_worldspace = (V * M *  vertPos).xyz;
    View = 0 - (V * M *vertPos).xyz;
    
    mode = normalShowing;
    LightDirection_cameraspace = lightPosition - position_worldspace;
    
    /* pass through the texture coordinates to be interpolated */
    vTexCoord = vertTex;
}