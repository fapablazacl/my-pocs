
#version 330

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

uniform vec4 uMaterialAmbient;
uniform vec4 uMaterialDiffuse;
uniform vec4 uMaterialSpecular;

uniform vec4 uLightAmbient;
uniform vec3 uLightDirection;
uniform vec4 uLightDiffuse;

in vec3 vertCoord;
in vec3 vertNormal;

out vec4 fragColor;

void main() {
    gl_Position = uProj * uView * uModel * vec4(vertCoord, 1.0);

    // compute modelView matrix and transform the normal with its inverse
    mat4 modelView = uView * uModel;
    // mat4 normalSpace = transpose(inverse(modelView));
    // vec3 normal = (normalSpace * vec4(vertNormal, 0.0)).xyz;
    vec3 normal = vertNormal;

    // compute the diffuse factor contribution, per vertex
    float d = abs(dot(uLightDirection, normal));

    vec4 ambient = uMaterialAmbient * uLightAmbient;
    vec4 diffuse = uMaterialDiffuse * uLightDiffuse * d;

    fragColor = ambient + diffuse;
    // fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
