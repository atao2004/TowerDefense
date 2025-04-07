#version 330

uniform sampler2D screen_texture;
uniform float hp_percentage;
uniform float exp_percentage;
uniform bool game_over;
uniform float darken_factor;

in vec2 texcoord;

layout(location = 0) out vec4 color;

// Kung: This code renders the HP and EXP bars
vec4 hp_exp_bars(vec4 in_color) 
{
	// Check to see whether the game is over and set the starting point for the experience bars
	if (texcoord[0] > 0.725 && !game_over) {
		// HP bar
		if (texcoord[1] > 0.925 && texcoord[1] < 0.975) {
			// Change the size of the bar depending on the hp value
			if (texcoord[0] < (0.725 + hp_percentage * 0.25)) {
				// Change colour if near 0 percent
				if (hp_percentage <= 0.2) {
					in_color = vec4(0.5, 0, 0, 0);
				} else in_color = vec4(1, 0, 0, 0);
			}
		}
		// EXP bar
		else if (texcoord[1] > 0.85 && texcoord[1] < 0.9) {
			// Change the size of the bar depending on the exp value
			if (texcoord[0] < (0.725 + exp_percentage * 0.25)) {
				// Change colour if near 100 percent
				if (exp_percentage >= 0.9) {
					in_color = vec4(0, 0, 0.7, 0);
				} else in_color = vec4(0.2, 0.2, 1, 0);
			}
		} 
			in_color -= darken_factor * vec4(0.8, 0.8, 0.8, 0);
		
	} else {
			in_color -= darken_factor * vec4(0.8, 0.8, 0.8, 0);
	}
	return in_color;
}

void main()
{
    vec4 in_color = texture(screen_texture, texcoord);
	color = hp_exp_bars(in_color);
}