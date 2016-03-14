#version 330 core
uniform sampler2D Texture2;

in vec2 vTexCoord;
in vec3 vColor;
in vec3 fragNor;
in vec3 lightDir;
in vec3 View;

out vec4 Outcolor;

void main() {
    vec3 normal = normalize(fragNor);
    vec3 lightNormal = normalize(lightDir);
    
    //vec3 MaterialDiffuseColor = vec3(0.0, 0.6, 1.0);
    vec3 LightColor = vec3(1.0, 1.0, 1.0);
    float cosTheta = max(dot(lightNormal, normal), 0);
    
    //vec3 R = -lightNormal + 2 * dot(normal, lightNormal) * normal;
    //vec3 View2 = normalize(View);
    //float cosBeta = max(dot(View2, R), 0);
    
   // vec4 BaseColor = vec4(vColor*LightColor + vColor * LightColor * cosTheta + pow(cosBeta, 20) * vColor * LightColor, 1.0);
    
    vec4 texColor2 = texture(Texture2, vTexCoord);
 
	Outcolor = vec4(texColor2.r*vColor.r, texColor2.g*vColor.g, texColor2.b*vColor.b, 1);
}

