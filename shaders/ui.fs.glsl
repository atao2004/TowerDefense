#version 330

uniform sampler2D screen_texture;
uniform float hp_percentage;
uniform float exp_percentage;
uniform bool game_over;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec4 hp_exp_bars(vec4 in_color) 
{
	if (texcoord[0] > 0.725 && !game_over) {
		if (texcoord[1] > 0.925 && texcoord[1] < 0.975) {
			if (texcoord[0] < (0.725 + hp_percentage * 0.25)) {
				if (hp_percentage <= 0.2) {
					in_color = vec4(0.5, 0, 0, 0);
				} else in_color = vec4(1, 0, 0, 0);
			}
		} else if (texcoord[1] > 0.85 && texcoord[1] < 0.9) {
			if (texcoord[0] < (0.725 + exp_percentage * 0.25)) {
				if (exp_percentage >= 0.9) {
					in_color = vec4(0, 0, 0.7, 0);
				} else in_color = vec4(0.2, 0.2, 1, 0);
			}
		}
	}
	return in_color;
}

void main()
{
    vec4 in_color = texture(screen_texture, texcoord);
	color = in_color;
	color = hp_exp_bars(color);
}