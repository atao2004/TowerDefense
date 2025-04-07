#version 330 core
in vec2 TexCoords;
in vec4 ParticleColor;
out vec4 color;

uniform sampler2D sprite;
uniform int particleType;

// void main()
// {
//     // For a custom soft particle without texture:
//     // Calculate distance from center
//     vec2 center = vec2(0.5, 0.5);
//     float dist = length(TexCoords - center) * 2.0;
    
//     // Create a soft circular particle
//     float alpha = 1.0 - smoothstep(0.0, 1.0, dist);
    
//     // Apply color
//     color = ParticleColor;
//     color.a *= alpha;
    
//     // Discard very transparent fragments
//     if(color.a < 0.01)
//         discard;
    
//     // If you want to use an actual texture:
//     // color = texture(sprite, TexCoords) * ParticleColor;
// }

void main()
{
    vec2 center = vec2(0.5, 0.5);
    
    // For electricity particles (type 1)
    if (particleType == 1) 
    {
        // Distance from center line (for line-like appearance)
        float dist = abs(TexCoords.y - 0.5) * 2.0;
        
        // Create a sharp line effect with soft edges
        float alpha = 1.0 - smoothstep(0.0, 0.3, dist);
        
        // Core brightness and outer glow
        float core = 1.0 - smoothstep(0.0, 0.1, dist);
        float glow = 1.0 - smoothstep(0.0, 0.8, dist);
        
        // Apply color with glow effect
        color = ParticleColor;
        
        // Brighten the center for a hot core
        color.rgb *= 1.0 + core * 0.5;
        color.a *= alpha;
        
        // Add glow effect
        color.rgb += ParticleColor.rgb * glow * 0.4;
        
        // Taper the ends to create continuous lines
        float taper = min(smoothstep(0.0, 0.2, TexCoords.x), smoothstep(1.0, 0.8, TexCoords.x));
        color.a *= taper;
    }
    else // Regular particles (blood, fire, etc.)
    {
        // For a custom soft particle without texture:
        // Calculate distance from center
        float dist = length(TexCoords - center) * 2.0;
        
        // Create a soft circular particle
        float alpha = 1.0 - smoothstep(0.0, 1.0, dist);
        
        // Apply color
        color = ParticleColor;
        color.a *= alpha;
    }
    
    // Discard very transparent fragments
    if(color.a < 0.01)
        discard;
}