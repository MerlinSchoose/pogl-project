#version 450

uniform mat4 model_view_matrix;
uniform mat4 projection_matrix;

in vec3 position;
in vec2 uv;

out float depth;

out vec2 vUv;

vec3 light_color = vec3(1.0, 1.0, 1.0);
vec3 light_position = vec3(1.0, 1.0, 5.0);

void main() {
    gl_Position = projection_matrix * model_view_matrix * vec4(position, 1.0);

    depth = position.y;
	//texture
    vUv = uv;
}