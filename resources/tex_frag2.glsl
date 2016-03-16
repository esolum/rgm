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
    vec4 LightColor = vec4(0.75, 0.75, 0.75, 1.0);
    float cosTheta = max(dot(lightNormal, normal), 0);
    
    vec3 R = -lightNormal + 2 * dot(normal, lightNormal) * normal;
    vec3 View2 = normalize(View);
    float cosBeta = max(dot(View2, R), 0);
    
    vec4 AmbMute = vec4(0.1, 0.1, 0.1, 1.0);
    
    vec4 texColor2 = texture(Texture2, vTexCoord);
    
    vec4 BaseColor = AmbMute*texColor2*LightColor + texColor2 * LightColor * cosTheta +
        pow(cosBeta, 120) * vec4(0.1, 0.1, 0.1, 1.0) * LightColor;
    Outcolor = BaseColor;
 
	//Outcolor = vec4(texColor2.r*BaseColor.r, texColor2.g*BaseColor.g, texColor2.b*BaseColor.b, 1);
    
}

