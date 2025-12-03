#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 VertexColor;

out vec4 FragColor;

uniform vec3 viewPos;
uniform vec4 ambientColor;
uniform vec4 meshColor;
uniform bool uHasTexture;
uniform sampler2D uTextureSampler;
uniform float time;

#define MAX_LIGHTS 8
struct Light {
    int type;
    vec3 position;
    vec3 direction;
    vec4 color;
    float intensity;
    float range;
    float spotAngle;
    float constant;
    float linear;
    float quadratic;
};
uniform Light lights[MAX_LIGHTS];
uniform int numLights;

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 ambient = ambientColor.rgb * ambientColor.a;
    vec3 result = ambient;
    
    for (int i = 0; i < numLights && i < MAX_LIGHTS; i++) {
        vec3 lightDir;
        float attenuation = 1.0;
        if (lights[i].type == 0) {
            lightDir = normalize(-lights[i].direction);
        } else if (lights[i].type == 1) {
            vec3 toLight = lights[i].position - FragPos;
            float dist = length(toLight);
            lightDir = normalize(toLight);
            attenuation = clamp(1.0 - dist / lights[i].range, 0.0, 1.0);
            attenuation *= attenuation;
        } else {
            vec3 toLight = lights[i].position - FragPos;
            float dist = length(toLight);
            lightDir = normalize(toLight);
            float theta = dot(lightDir, normalize(-lights[i].direction));
            float cutoff = cos(radians(lights[i].spotAngle));
            if (theta > cutoff) {
                attenuation = clamp(1.0 - dist / lights[i].range, 0.0, 1.0);
                attenuation *= attenuation;
                attenuation *= (theta - cutoff) / (1.0 - cutoff);
            } else {
                attenuation = 0.0;
            }
        }

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lights[i].color.rgb * lights[i].intensity;
        
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0);
        vec3 specular = spec * lights[i].color.rgb * lights[i].intensity * 0.5;
        result += (diffuse + specular) * attenuation;
    }

    if (numLights == 0) {
        result = vec3(1.0);
    }

    vec4 baseColor = meshColor * VertexColor;
    if (uHasTexture) {
        vec4 texColor = texture(uTextureSampler, TexCoord);
        baseColor = baseColor * texColor;
    }
    
    // Add rainbow effect based on time and position
    vec3 rainbow = vec3(
        sin(time + FragPos.x * 2.0) * 0.5 + 0.5,
        sin(time + FragPos.y * 2.0 + 2.094) * 0.5 + 0.5,
        sin(time + FragPos.z * 2.0 + 4.189) * 0.5 + 0.5
    );
    
    FragColor = vec4(result * baseColor.rgb * rainbow, baseColor.a);
}