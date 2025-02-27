#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform bool player_is_hit;  // For hit animation

// Output color
layout(location = 0) out vec4 color;

void main()
{
    color = texture(sampler0, vec2(texcoord.x, texcoord.y));
    if (player_is_hit) {
        color.rgb = mix(color.rgb, vec3(1.0f ,0.0f ,0.0f), 0.7);  // Mix with red
    }
}