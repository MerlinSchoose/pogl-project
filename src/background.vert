#version 450

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;
uniform vec3 cameraPos;

in vec3 position;

out float depth;
out float cameraDepth;
out vec3 ray;

void main() {
    gl_Position = projection_matrix * model_view_matrix * vec4(position, 1.0);

    ray = position - cameraPos;
    depth = position.y;
    cameraDepth = cameraPos.y;
}
