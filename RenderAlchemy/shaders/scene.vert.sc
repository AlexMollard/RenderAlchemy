$input a_position, a_normal, a_color
$output v_color, v_fragPos, v_normal

#include <bgfx_shader.sh>

void main() {
    v_color = a_color;
    v_fragPos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
    v_normal = mul(u_model[0], vec4(a_normal, 0.0)).xyz;
    
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}
