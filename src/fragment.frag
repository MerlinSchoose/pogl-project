#version 450

uniform sampler2D caustic_sampler;
uniform sampler2D floor_sampler;

in vec2 vUv;
in float depth;

const float FOG_MIN = .6;
const float FOG_MAX = 1.;
const float black_depth = -1000;
const vec4 fog_color = vec4(.1, .55, .7, 1);

layout(location=0) out vec4 output_color;

float fog(float depth) {
    return clamp(1 - (FOG_MAX - depth) / (FOG_MAX - FOG_MIN), 0, 1);
}

void main() {
    vec4 floor_color = mix(texture(floor_sampler, vUv * 100), vec4(vec3(texture(caustic_sampler, vUv * 50).r), 1), .5);

    float z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    float fogv = fog(z);
    output_color = mix(floor_color, fog_color, fogv * fogv * fogv);
    //outputcolor = vec4(vec3(z), 1);
    float fogv2 = clamp(depth / black_depth, 0, 1);
    output_color = mix(output_color, vec4(vec3(0), 1), fogv2 * fogv2);
}