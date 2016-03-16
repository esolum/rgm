#version 330 core
uniform sampler2D Texture1;

in vec2 vTexCoord;
in vec3 vColor;
in vec3 fragNor;
in vec3 lightDir;
in vec3 View;

out vec4 Outcolor;

void main() {
    
    //Fragment shader for old world
    
    vec3 normal = normalize(fragNor);
    vec3 lightNormal = normalize(lightDir);
    
    //vec3 MaterialDiffuseColor = vec3(0.0, 0.6, 1.0);
    vec3 LightColor = vec3(0.75, 0.75, 0.75);
    float cosTheta = max(dot(lightNormal, normal), 0);
    
    vec3 R = -lightNormal + 2 * dot(normal, lightNormal) * normal;
    vec3 View2 = normalize(View);
    float cosBeta = max(dot(View2, R), 0);
    
    vec4 BaseColor = vec4(vColor*LightColor + vColor * LightColor * cosTheta + pow(cosBeta, 20) * vColor * LightColor, 1.0);

    vec4 texColor1 = texture(Texture1, vTexCoord);
 
    Outcolor = vec4(texColor1.r*BaseColor.r, texColor1.g*BaseColor.g, texColor1.b*BaseColor.b, 1);
  //if(texColor1.r < 0.4 && texColor1.g < 0.4 && texColor1.b > 0.35) {
  	 //Outcolor = vec4(texColor1.r*vColor.r, texColor1.g*vColor.g, texColor1.b*vColor.b, 0.35);
  //}
  
}

