#version 330 core
uniform sampler2D Texture0;

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
    vec3 LightColor = vec3(0.75, 0.75, 0.75);
    float cosTheta = max(dot(lightNormal, normal), 0);
    
    vec3 R = -lightNormal + 2 * dot(normal, lightNormal) * normal;
    vec3 View2 = normalize(View);
    float cosBeta = max(dot(View2, R), 0);
    
    vec4 BaseColor = vec4(vColor*LightColor + vColor * LightColor * cosTheta + pow(cosBeta, 20) * vColor * LightColor, 1.0);

  vec4 texColor0 = texture(Texture0, vTexCoord);
 
  	Outcolor = vec4(texColor0.r*BaseColor.r, texColor0.g*BaseColor.g, texColor0.b*BaseColor.b, 1);
  //DEBUG:Outcolor = vec4(vTexCoord.s, vTexCoord.t, 0, 1);
}

