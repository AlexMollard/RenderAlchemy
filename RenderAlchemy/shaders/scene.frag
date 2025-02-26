#version 330 core
in vec3 color;
in vec3 fragPos;
in vec3 normal;
out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform float ambientStrength;
uniform bool useTexturing;
uniform int sceneType;

void main() {
    // Ambient lighting
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse lighting
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * lightIntensity;
    
    // Varying color based on scene type
    vec3 baseColor;
    if (sceneType == 0) {
        // Use vertex colors for cube
        baseColor = color;
    } else if (sceneType == 1) {
        // Create gradient for sphere
        baseColor = mix(vec3(1.0, 0.2, 0.1), vec3(0.1, 0.5, 1.0), (fragPos.y + 1.0) / 2.0);
    } else {
        // Create checkerboard pattern for plane
        float scale = 4.0;
        int ix = int(floor(fragPos.x * scale));
        int iz = int(floor(fragPos.z * scale));
        if ((ix + iz) % 2 == 0) {
            baseColor = vec3(0.9, 0.9, 0.9);
        } else {
            baseColor = vec3(0.2, 0.2, 0.2);
        }
    }
    
    // Apply lighting to base color
    vec3 result = (ambient + diffuse) * baseColor;
    
    // Output HDR color values
    FragColor = vec4(result, 1.0);
}