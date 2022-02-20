
#version 330

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

in vec3 vertCoord;
in vec3 vertNormal;
in vec2 vertTexCoord;

out vec3 fragNormal;
out vec2 fragTexCoord;

void main() {
    gl_Position = uProj * uView * uModel * vec4(vertCoord, 1.0);

    // compute modelView matrix and transform the normal with its inverse
    // mat4 modelView = uView * uModel;
    // mat4 normalSpace = transpose(inverse(modelView));
    // vec3 normal = (normalSpace * vec4(vertNormal, 0.0)).xyz;
    fragNormal = (transpose(inverse(uModel)) * vec4(vertNormal, 0.0)).xyz;
    fragTexCoord = vertTexCoord;
}
