#version 330

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

in vec3 vertCoord;

out vec4 fragColor;

void main() {
    gl_Position = uProj * uView * uModel * vec4(vertCoord, 1.0);

    fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
