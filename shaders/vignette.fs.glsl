#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;
uniform float hp_percentage;
uniform float exp_percentage;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec4 hp_exp_bars(vec4 in_color) 
{
	if (texcoord[0] > 0.725) {
		if (texcoord[1] > 0.925 && texcoord[1] < 0.975) {
			if (texcoord[0] < (0.725 + hp_percentage * 0.25)) in_color = vec4(1, 0, 0, 0);
		} else if (texcoord[1] > 0.85 && texcoord[1] < 0.9) {
			if (texcoord[0] < (0.725 + exp_percentage * 0.25)) in_color = vec4(0.3, 0.3, 1, 0);
		}
	}
	return in_color;
}

// darken the screen, i.e., fade to black
vec4 fade_color(vec4 in_color) 
{
	if (darken_screen_factor > 0)
		in_color -= darken_screen_factor * vec4(0.8, 0.8, 0.8, 0);
	return in_color;
}

void main()
{
    vec4 in_color = texture(screen_texture, texcoord);
    color = fade_color(in_color);
	color = hp_exp_bars(color);
}