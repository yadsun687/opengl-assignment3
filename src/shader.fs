#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 TangentPlayerLightPos;
} fs_in;

float p_constant = 1.0;
float p_linear = 0.14;
float p_quadratic = 0.07;
vec3 p_ambient = vec3(1.0, 0.57, 0.16);
vec3 p_diffuse = vec3(0.8);
vec3 p_specular = vec3(0.3);

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

uniform bool hasNormalMap;

const bool enablePixelation = true;

vec3 CalcPlayerPointLight(vec3 normal, vec3 diffuseColor)
{
    vec3 lightDir = normalize(fs_in.TangentPlayerLightPos - fs_in.TangentFragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    // attenuation
    float distance = length(fs_in.TangentPlayerLightPos - fs_in.TangentFragPos);
    float attenuation = 1.0 / (p_constant + p_linear * distance +
                p_quadratic * (distance * distance));

    // combine results
    vec3 ambient = p_ambient * diffuseColor;
    vec3 diffuse = p_diffuse * diff * diffuseColor;
    vec3 specular = p_specular * spec * vec3(0.2);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec2 pixelateUV(vec2 uv, float resolution) {
    return floor(uv * resolution) / resolution;
}

vec3 posterize(vec3 color, float levels) {
    return floor(color * levels) / levels;
}

void main()
{
    vec2 texCoords = enablePixelation ? pixelateUV(fs_in.TexCoords, 128.0) : fs_in.TexCoords;

    vec3 normal;
    if (hasNormalMap) {
        // obtain normal from normal map in range [0,1]
        normal = texture(texture_normal1, texCoords).rgb;
        // transform normal vector to range [-1,1]
        normal = normal * 2.0 - 1.0; // this normal is in tangent space
    } else {
        // no normal map -> flat surface
        normal = vec3(0.0, 0.0, 1.0);
    }

    vec3 lightColor = vec3(0.015, 0.101, 0.25);
    // get diffuse color
    // ambient
    vec3 ambient = 0.2 * lightColor;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = 0.2 * spec * lightColor;

    vec3 color = texture(texture_diffuse1, texCoords).rgb;
    vec3 totalLight = CalcPlayerPointLight(normal, color) + ((ambient + diffuse + specular) * color);

    // FragColor = vec4(texCoords.xy , 0.0, 1.0); // uv color
    // FragColor = vec4(lightDir * 0.5 + 0.5, 1.0); // Visualize light direction
    // FragColor = vec4(normal * 0.5 + 0.5, 1.0); //normal map color
    // FragColor = vec4(vec3(diff), 1.0); //light intensity
    // FragColor = vec4(ambient + diffuse + specular, 1.0);
    //
    // Apply color posterization for retro look
    if (enablePixelation) {
        totalLight = posterize(totalLight, 16);
    }

    FragColor = vec4(totalLight, 1.0);
}
