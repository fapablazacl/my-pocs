#version 330

in vec3 fragNormal;
in vec2 fragTexCoord;

uniform float uMaterialDiffuseSamplerEnabled = 1.0;
uniform sampler2D uMaterialDiffuseSampler;

uniform vec4 uMaterialAmbient;
uniform vec4 uMaterialDiffuse;
uniform vec4 uMaterialSpecular;

uniform vec4 uGlobalLightAmbient = vec4(0.1, 0.1, 0.1, 1.0);

uniform vec3 uLightDirection;
uniform vec4 uLightAmbient;
uniform vec4 uLightDiffuse;

out vec4 finalColor;

void main() {
    vec3 normal = fragNormal;

    // compute the diffuse factor contribution, per vertex
    float d = max(dot(uLightDirection, normal), 0.0);

    vec4 ambient = uGlobalLightAmbient + uMaterialAmbient * uLightAmbient;
    // vec4 diffuse = (uMaterialDiffuseSamplerEnabled == 1.0 ? texture(uMaterialDiffuseSampler, fragTexCoord) : uMaterialDiffuse) * uLightDiffuse * d;

    vec4 diffuse = texture(uMaterialDiffuseSampler, fragTexCoord);

    // finalColor = vec4(fragTexCoord, 1.0, 1.0) * d;
    // finalColor = texture(uMaterialDiffuseSampler, fragTexCoord);

    finalColor = ambient + diffuse;
}
