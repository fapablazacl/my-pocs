
precision mediump float;

attribute vec3 vCoord;
attribute vec3 vColor;

varying vec3 color;

void main() {
    gl_Position = vec4(vCoord, 1.0);
    color = vColor;
}
