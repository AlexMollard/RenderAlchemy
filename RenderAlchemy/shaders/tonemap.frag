#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D hdrBuffer;
uniform sampler1D colorLUT1D;
uniform sampler3D colorLUT3D;
uniform float exposure;
uniform float clutStrength;
uniform int tonemapOperator;
uniform bool splitScreen;
uniform float splitPosition;
uniform bool showClut;
uniform bool applyClut;
uniform bool use3DCLUT;
uniform int clutSize;

vec3 reinhardTonemap(vec3 hdrColor) {
    return hdrColor / (hdrColor + vec3(1.0));
}

vec3 acesTonemap(vec3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 apply1DCLUT(vec3 color) {
    // Compute luminance to index the CLUT
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    
    // Make sure luminance is in valid range [0,1]
    luminance = clamp(luminance, 0.0, 1.0);
    
    // Sample the CLUT
    vec3 clutColor = texture(colorLUT1D, luminance).rgb;
    
    // Blend between original and CLUT mapped colors
    return mix(color, clutColor, clutStrength);
}

vec3 apply3DCLUT(vec3 color) {
    // Scale the texture coordinates to [0, 1]
    color = clamp(color, 0.0, 1.0);
    
    // Adjust the texture coordinates to sample at the center of each texel
    float size = float(clutSize);
    float halfPixel = 0.5 / size;
    
    // Scale and offset to properly sample the 3D texture
    vec3 scale = vec3((size - 1.0) / size);
    vec3 offset = vec3(halfPixel);
    
    // Sample the 3D CLUT with corrected coordinates
    vec3 clutColor = texture(colorLUT3D, color * scale + offset).rgb;
    
    // Blend between original and CLUT mapped colors
    return mix(color, clutColor, clutStrength);
}

void main() {
    // Get position for split screen logic
    float screenPosition = TexCoord.x;
    
    // Get HDR color from the framebuffer
    vec3 hdrColor = texture(hdrBuffer, TexCoord).rgb;
    
    // Apply tone mapping based on selected operator
    vec3 mapped;
    if (tonemapOperator == 0) {
        mapped = reinhardTonemap(hdrColor * exposure);
    } else {
        mapped = acesTonemap(hdrColor * exposure);
    }
    
    // Apply color grading with CLUT if enabled
    vec3 finalColor = mapped;
    if (applyClut) {
        if (use3DCLUT) {
            finalColor = apply3DCLUT(mapped);
        } else {
            finalColor = apply1DCLUT(mapped);
        }
    }
    
    // Split screen visualization if enabled
    if (splitScreen && screenPosition > splitPosition) {
        finalColor = mapped; // Show without CLUT on right side
    }
    
    // For 1D CLUT visualization
    if (showClut && !use3DCLUT && TexCoord.y < 0.05 && TexCoord.x >= 0.05 && TexCoord.x <= 0.95) {
        // Scale x position to sample the CLUT
        float clutPos = clamp((TexCoord.x - 0.05) / 0.9, 0.0, 1.0);
        finalColor = texture(colorLUT1D, clutPos).rgb;
    }
    
    // For 3D CLUT visualization (shows a slice)
    if (showClut && use3DCLUT && TexCoord.y < 0.1 && TexCoord.x < 0.3) {
        // Create a visualization of the 3D CLUT 
        // (using x and y to show a slice at a fixed blue level)
        vec3 clutCoord;
        clutCoord.r = clamp(TexCoord.x / 0.3, 0.0, 1.0);
        clutCoord.g = clamp(TexCoord.y / 0.1, 0.0, 1.0);
        clutCoord.b = 0.5; // middle slice for visualization
        
        // Adjust for texel centers
        float size = float(clutSize);
        float halfPixel = 0.5 / size;
        vec3 scale = vec3((size - 1.0) / size);
        vec3 offset = vec3(halfPixel);
        
        finalColor = texture(colorLUT3D, clutCoord * scale + offset).rgb;
    }
    
    FragColor = vec4(finalColor, 1.0);
}