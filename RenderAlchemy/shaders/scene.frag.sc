$input v_color, v_fragPos, v_normal

#include <bgfx_shader.sh>

uniform vec4 u_lightPos;
uniform vec4 u_lightColor;
uniform vec4 u_params; // x = lightIntensity, y = ambientStrength, z = sceneType

void main() {
    // Ambient lighting
    vec3 ambient = u_params.y * u_lightColor.rgb;
    
    // Diffuse lighting
    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(u_lightPos.xyz - v_fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_lightColor.rgb * u_params.x;
    
    // Varying color based on scene type
    vec3 baseColor;
    int sceneType = int(u_params.z);
    
    if (sceneType == 0) {
        // Use vertex colors for cube
        baseColor = v_color;
    } else if (sceneType == 1) {
        // Create gradient for sphere
        baseColor = mix(vec3(1.0, 0.2, 0.1), vec3(0.1, 0.5, 1.0), (v_fragPos.y + 1.0) / 2.0);
    } else {
        // Create checkerboard pattern for plane
        float scale = 4.0;
        int ix = int(floor(v_fragPos.x * scale));
        int iz = int(floor(v_fragPos.z * scale));
        if ((ix + iz) % 2 == 0) {
            baseColor = vec3(0.9, 0.9, 0.9);
        } else {
            baseColor = vec3(0.2, 0.2, 0.2);
        }
    }
    
    // Apply lighting to base color
    vec3 result = (ambient + diffuse) * baseColor;
    
    // Output HDR color values
    gl_FragColor = vec4(result, 1.0);
}
