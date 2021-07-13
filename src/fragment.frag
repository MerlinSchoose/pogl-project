#version 450

uniform sampler2D caustic_sampler;
uniform sampler2D floor_sampler;

in vec2 vUv;
in vec4 center;

const float FOG_MIN = .55;
const float FOG_MAX = 0.7;

layout(location=0) out vec4 output_color;

float fog(float depth) {
    return clamp(1 - (FOG_MAX - depth) / (FOG_MAX - FOG_MIN), 0, 1);
}

void main() {
    output_color = mix(texture(floor_sampler, vUv), vec4(vec3(texture(caustic_sampler, vUv).r), 1), .5);
}