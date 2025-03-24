#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Instance attributes
in vec4 instance_pos_scale; // xy = position, zw = scale
in vec4 instance_color;     // rgba color
in vec4 instance_life_data; // x = life_ratio, yzw = reserved

// Output to fragment shader
out vec2 TexCoords;
out vec4 ParticleColor;
out float life_ratio;

// Transformation matrices
uniform mat3 projection;

void main()
{
    // Transform position by instance attributes
    vec2 pos = in_position.xy * instance_pos_scale.zw;
    vec3 screen_pos = projection * vec3(pos + instance_pos_scale.xy, 1.0);
    
    // Pass values to fragment shader
    TexCoords = in_texcoord;
    ParticleColor = instance_color;
    life_ratio = instance_life_data.x;
    
    gl_Position = vec4(screen_pos.xy, 0.0, 1.0);
}