#version 330

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

in vec3 vertCoord;
in vec4 vertColor;

out vec4 fragColor;

void main() {
    gl_Position = uProj * uView * uModel * vec4(vertCoord, 1.0);

    fragColor = vertColor;
}
