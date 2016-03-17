#version 330 core

uniform float lightColor2;
uniform float lightColor3;
uniform float lightColor4;
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 Spec;
uniform float Shine;
in vec3 fragNor;
in float mode;
in vec3 View;
in vec2 vTexCoord;
in vec3 position;
in vec3 lightDir1;
in vec3 lightDir2;
in vec3 lightDir3;
in vec3 lightDir4;
in float att1;
in float att2;
in float att3;
in float att4;
out vec4 color;

void main()
{
    vec3 white = vec3(1.0, 1.0, 1.0);
    vec3 lightSwitch2 = vec3(lightColor2, lightColor2, lightColor2);
    vec3 lightSwitch3 = vec3(lightColor3, lightColor3, lightColor3);
    vec3 lightSwitch4 = vec3(lightColor4, lightColor4, lightColor4);
    vec4 color1;
    vec4 color2;
    vec4 color3;
    vec4 color4;
    
    //Apparently GLSL doesn't want to read my function so I have to do this the hard way.
    //Compute the color attenuation for point light 1
    
    vec3 vColor = vec3(max(dot(fragNor, normalize(lightDir1)), 0));
    
    
    vec3 normal = normalize(fragNor);
    vec3 lightNormal = normalize(lightDir1);
    
    float cosTheta = max(dot(lightNormal, normal), 0);
    
    
    vec3 R = -lightNormal + 2 * dot(normal, lightNormal) * normal;
    vec3 View2 = normalize(View);
    float cosBeta = max(dot(View2, R), 0);
    
    
    color1 = vec4(MatAmb*white + MatDif * white * cosTheta + pow(cosBeta, Shine) * Spec * white, 1.0);
    color1 = vec4(color1.r * att1, color1.g * att1, color1.b * att1, 1.0);
    
    
    //POINT LIGHT 2
    vColor = vec3(max(dot(fragNor, normalize(lightDir2)), 0));

    
    normal = normalize(fragNor);
    lightNormal = normalize(lightDir2);
    
    cosTheta = max(dot(lightNormal, normal), 0);
    
    
    R = -lightNormal + 2 * dot(normal, lightNormal) * normal;
    View2 = normalize(View);
    cosBeta = max(dot(View2, R), 0);
   
    color2 = vec4(MatAmb*lightColor2  + MatDif * lightColor2 * cosTheta + pow(cosBeta, Shine) * Spec * lightColor2, 1.0);
    color2 = vec4(color2.r * att2, color2.g * att2, color2.b * att2, 1.0);
    
    //POINT LIGHT 3
    vColor = vec3(max(dot(fragNor, normalize(lightDir3)), 0));
    
    
    normal = normalize(fragNor);
    lightNormal = normalize(lightDir3);
    
    cosTheta = max(dot(lightNormal, normal), 0);
    
    
    R = -lightNormal + 2 * dot(normal, lightNormal) * normal;
    View2 = normalize(View);
    cosBeta = max(dot(View2, R), 0);
    
    color3 = vec4(MatAmb*lightColor3 + MatDif * lightColor3 * cosTheta + pow(cosBeta, Shine) * Spec * lightColor3, 1.0);
    color3 = vec4(color3.r * att3, color3.g * att3, color3.b * att3, 1.0);
    
    //POINT LIGHT 4
    vColor = vec3(max(dot(fragNor, normalize(lightDir4)), 0));
    
    normal = normalize(fragNor);
    lightNormal = normalize(lightDir4);
    
    cosTheta = max(dot(lightNormal, normal), 0);
    
    
    R = -lightNormal + 2 * dot(normal, lightNormal) * normal;
    View2 = normalize(View);
    cosBeta = max(dot(View2, R), 0);
    
    color4 = vec4(MatAmb*lightColor4 + MatDif * lightColor4 * cosTheta + pow(cosBeta, Shine) * Spec * lightColor4, 1.0);
    color4 = vec4(color4.r * att4, color4.g * att4, color4.b * att4, 1.0);

    color = color1 + color2 + color3 + color4;
    
    
    
}
