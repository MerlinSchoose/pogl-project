#version 450

uniform sampler2D texture_sampler;
uniform sampler2D lighting_sampler;
uniform sampler2D normalmap_sampler;

in vec2 vUv;
in vec4 center;

const float FOG_MIN = .55;
const float FOG_MAX = 0.7;

layout(location=0) out vec4 output_color;

float fog(float depth) {
    return clamp(1 - (FOG_MAX - depth) / (FOG_MAX - FOG_MIN), 0, 1);
}

void main() {
    output_color = texture(texture_sampler, vUv);
}