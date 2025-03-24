#version 330 core
in vec2 TexCoords;
in vec4 ParticleColor;
out vec4 color;

uniform sampler2D sprite;

void main()
{
    // For a custom soft particle without texture:
    // Calculate distance from center
    vec2 center = vec2(0.5, 0.5);
    float dist = length(TexCoords - center) * 2.0;
    
    // Create a soft circular particle
    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);
    
    // Apply color
    color = ParticleColor;
    color.a *= alpha;
    
    // Discard very transparent fragments
    if(color.a < 0.01)
        discard;
    
    // If you want to use an actual texture:
    // color = texture(sprite, TexCoords) * ParticleColor;
}