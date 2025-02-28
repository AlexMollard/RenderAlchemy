$input v_texcoord0

#include <bgfx_shader.sh>

// Uniforms
uniform vec4 u_params;          // x: exposure, y: clutStrength, z: tonemapOperator, w: splitPosition
uniform vec4 u_flags;           // x: splitScreen, y: showClut, z: applyClut, w: use3DCLUT
uniform vec4 u_clutParams;      // x: clutSize

SAMPLER2D(s_hdrBuffer, 0);
SAMPLER1D(s_colorLUT1D, 1);
SAMPLER3D(s_colorLUT3D, 2);

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
    vec3 clutColor = texture2DLod(s_colorLUT1D, vec2(luminance, 0.0), 0.0).rgb;
    
    // Blend between original and CLUT mapped colors
    return mix(color, clutColor, u_params.y); // clutStrength
}

vec3 apply3DCLUT(vec3 color) {
    // Scale the texture coordinates to [0, 1]
    color = clamp(color, 0.0, 1.0);
    
    // Adjust the texture coordinates to sample at the center of each texel
    float size = u_clutParams.x;
    float halfPixel = 0.5 / size;
    
    // Scale and offset to properly sample the 3D texture
    vec3 scale = vec3((size - 1.0) / size);
    vec3 offset = vec3(halfPixel);
    
    // Sample the 3D CLUT with corrected coordinates
    vec3 clutColor = texture3DLod(s_colorLUT3D, color * scale + offset, 0.0).rgb;
    
    // Blend between original and CLUT mapped colors
    return mix(color, clutColor, u_params.y); // clutStrength
}

void main() {
    // Get position for split screen logic
    float screenPosition = v_texcoord0.x;
    
    // Get HDR color from the framebuffer
    vec3 hdrColor = texture2D(s_hdrBuffer, v_texcoord0).rgb;
    
    // Apply tone mapping based on selected operator
    vec3 mapped;
    if (u_params.z < 0.5) { // tonemapOperator == 0
        mapped = reinhardTonemap(hdrColor * u_params.x); // exposure
    } else {
        mapped = acesTonemap(hdrColor * u_params.x); // exposure
    }
    
    // Apply color grading with CLUT if enabled
    vec3 finalColor = mapped;
    if (u_flags.z > 0.5) { // applyClut
        if (u_flags.w > 0.5) { // use3DCLUT
            finalColor = apply3DCLUT(mapped);
        } else {
            finalColor = apply1DCLUT(mapped);
        }
    }
    
    // Split screen visualization if enabled
    if (u_flags.x > 0.5 && screenPosition > u_params.w) { // splitScreen && screenPosition > splitPosition
        finalColor = mapped; // Show without CLUT on right side
    }
    
    // For 1D CLUT visualization
    if (u_flags.y > 0.5 && u_flags.w < 0.5 && v_texcoord0.y < 0.05 && v_texcoord0.x >= 0.05 && v_texcoord0.x <= 0.95) {
        // showClut && !use3DCLUT
        // Scale x position to sample the CLUT
        float clutPos = clamp((v_texcoord0.x - 0.05) / 0.9, 0.0, 1.0);
        finalColor = texture2DLod(s_colorLUT1D, vec2(clutPos, 0.0), 0.0).rgb;
    }
    
    // For 3D CLUT visualization (shows a slice)
    if (u_flags.y > 0.5 && u_flags.w > 0.5 && v_texcoord0.y < 0.1 && v_texcoord0.x < 0.3) {
        // showClut && use3DCLUT
        // Create a visualization of the 3D CLUT 
        // (using x and y to show a slice at a fixed blue level)
        vec3 clutCoord;
        clutCoord.r = clamp(v_texcoord0.x / 0.3, 0.0, 1.0);
        clutCoord.g = clamp(v_texcoord0.y / 0.1, 0.0, 1.0);
        clutCoord.b = 0.5; // middle slice for visualization
        
        // Adjust for texel centers
        float size = u_clutParams.x;
        float halfPixel = 0.5 / size;
        vec3 scale = vec3((size - 1.0) / size);
        vec3 offset = vec3(halfPixel);
        
        finalColor = texture3DLod(s_colorLUT3D, clutCoord * scale + offset, 0.0).rgb;
    }
    
    gl_FragColor = vec4(finalColor, 1.0);
}
