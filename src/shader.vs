// #version 330 core
// layout(location = 0) in vec3 aPos;
// layout(location = 1) in vec3 aNormal;
// layout(location = 2) in vec2 aTexCoords;
// layout(location = 3) in vec3 aTangent;
// layout(location = 4) in vec3 aBitangent;

// out VS_OUT {
//     vec3 FragPos;
//     vec2 TexCoords;
//     vec3 TangentLightPos;
//     vec3 TangentViewPos;
//     vec3 TangentFragPos;
// } vs_out;

// uniform mat4 projection;
// uniform mat4 view;
// uniform mat4 model;

// uniform vec3 lightPos;
// uniform vec3 viewPos;

// out vec3 debug_normal;

// void main()
// {
//     debug_normal = aNormal;

//     vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
//     vs_out.TexCoords = aTexCoords;

//     mat3 normalMatrix = transpose(inverse(mat3(model)));
//     vec3 T = normalize(normalMatrix * aTangent);
//     vec3 N = normalize(normalMatrix * aNormal);
//     T = normalize(T - dot(T, N) * N);
//     vec3 B = cross(T, N);

//     mat3 TBN = transpose(mat3(T, B, N));
//     vs_out.TangentLightPos = TBN * lightPos;
//     vs_out.TangentViewPos = TBN * viewPos;
//     vs_out.TangentFragPos = TBN * vs_out.FragPos;

//     gl_Position = projection * view * model * vec4(aPos, 1.0);
// }

#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;
layout(location = 5) in ivec4 boneIds;
layout(location = 6) in vec4 weights;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 TangentPlayerLightPos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 playerLightPos;

//animation
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];
uniform bool hasAnimation;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(T, N);

    mat3 TBN = transpose(mat3(T, B, N));
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos = TBN * viewPos;
    vs_out.TangentFragPos = TBN * vs_out.FragPos;
    vs_out.TangentPlayerLightPos = TBN * playerLightPos;

    // transform from animation
    if (hasAnimation)
    {
        vec4 totalPosition = vec4(0.0f);
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            if (boneIds[i] == -1)
                continue;
            if (boneIds[i] >= MAX_BONES)
            {
                totalPosition = vec4(aPos, 1.0f);
                break;
            }
            //average by the bones weight
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0f);
            totalPosition += localPosition * weights[i];
        }

        gl_Position = projection * view * model * totalPosition;
    } else {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
}
