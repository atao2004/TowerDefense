#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;
uniform float hp_percentage;
uniform float exp_percentage;
uniform bool game_over;

in vec2 texcoord;
uniform vec2 tex_offset;

layout(location = 0) out vec4 color;

// darken the screen, i.e., fade to black
vec4 fade_color_lerp(vec4 in_color) 
{

    //M1 interpolation implementation
    //(1 - time) * vec4(0.1, 0.1, 0.1, 0) + vec4(0.5, 0.5, 0.5, 1) * time;
    //in_color = mix(in_color, B, time);
    if (time > 0 && game_over)
        //in_color = time/1000;
        in_color -=  (1 - time/5000)*vec4(0, 0, 0, 1) + vec4(1, 1, 1, 1)*(time/6000) + vec4(0, 1, 1, 0)*(time/1); 
    return in_color;
}

void main()
{
    // Shift the texture coordinates
    vec2 shifted_coord = texcoord + tex_offset;

    // Sample using the shifted coordinates
    vec4 in_color = texture(screen_texture, tex_offset);
    color = fade_color_lerp(in_color);
}