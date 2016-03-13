#version 330 core
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 Spec;
uniform float Shine;
in vec3 fragNor;
in vec3 LightDirection_cameraspace;
in float mode;
in vec3 View;
in vec3 vTexCoord;
out vec4 color;



void main()
{
    vec3 normal = normalize(fragNor);
    vec3 lightNormal = normalize(LightDirection_cameraspace);
    
    //vec3 MaterialDiffuseColor = vec3(0.0, 0.6, 1.0);
    vec3 LightColor = vec3(1.0, 1.0, 1.0);
    float cosTheta = max(dot(lightNormal, normal), 0);
    
    vec3 LightIntensity = vec3(1.0, 1.0, 1.0);
    vec3 R = -lightNormal + 2 * dot(normal, lightNormal) * normal;
    vec3 View2 = normalize(View);
    float cosBeta = max(dot(View2, R), 0);
    
    //vec3 MaterialAmbientColor = 0.15 * MatDif;
    
    //color = MatAmb + MatDif * LightColor * cosTheta;
    //color = MatDif * cosTheta * LightColor + MatAmb;
    
    color = vec4(MatAmb*LightColor + MatDif * LightColor * cosTheta + pow(cosBeta, Shine) * Spec * LightColor, 1.0);
    //color = vec4(MatDif * LightColor * cosTheta + MatAmb, 1.0);
    
    if (mode == 1) {
        vec3 Ncolor = 0.5*normal + 0.5;
        color = vec4(Ncolor, 1.0);
    }
    
    //color = vec4(vTexCoord.s, vTexCoord.t, 0, 1);
    
}
