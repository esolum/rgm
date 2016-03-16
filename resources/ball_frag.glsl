#version 330 core
//uniform vec3 MatAmb;
//uniform vec3 MatDif;
//uniform vec3 Spec;
//uniform float Shine;
uniform sampler2D Texture3;
in vec3 fragNor;
in vec3 lightDir;
in vec3 View;
in vec2 vTexCoord;
in vec3 vColor;
out vec4 color;




void main()
{
    vec3 normal = normalize(fragNor);
    vec3 lightNormal = normalize(lightDir);
    
    //vec3 MaterialDiffuseColor = vec3(0.0, 0.6, 1.0);
    vec3 LightColor = vec3(1.0, 1.0, 1.0);
    float cosTheta = max(dot(lightNormal, normal), 0);
    
    vec3 R = -lightNormal + 2 * dot(normal, lightNormal) * normal;
    vec3 View2 = normalize(View);
    float cosBeta = max(dot(View2, R), 0);
    
    vec4 texColor3 = texture(Texture3, vTexCoord);
    
    
    
    //color = vec4(MatAmb*LightColor + MatDif * LightColor * cosTheta + pow(cosBeta, Shine) * Spec * LightColor, 1.0);
    
    //vec4 BaseColor = vec4(MatAmb*LightColor, 1.0) + vec4(LightColor * MatDif * cosTheta + pow(cosBeta, 120) * Spec*LightColor, 1.0);
    
    color = vec4(texColor3.r*vColor.r, texColor3.g*vColor.g, texColor3.b*vColor.b, 1);
    
    //color = vec4(vTexCoord.s, vTexCoord.t, 0, 1);
    
}
