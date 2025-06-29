#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/tiny_ecs.hpp"


// fonts
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem
{
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
		std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CHICKEN, mesh_path("chicken.obj"))
		// specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators (see TEXTURE_ASSET_ID).
	const std::array<std::string, texture_count> texture_paths = {
		textures_path("towers/tower01.png"),
		textures_path("seeds/seed_0.png"),
		textures_path("seeds/seed_1.png"),
		textures_path("seeds/seed_2.png"),
		textures_path("seeds/seed_3.png"),
		textures_path("seeds/seed_4.png"),
		textures_path("seeds/seed_5.png"),
		textures_path("seeds/seed_6.png"),
		textures_path("seeds/seed_7.png"),
		textures_path("enemies/zombie_walk1.png"),
		textures_path("enemies/zombie_walk2.png"),
		textures_path("enemies/zombie_spawn1.png"),
		textures_path("enemies/zombie_spawn2.png"),
		textures_path("player/idle.png"),
		textures_path("player/walk1.png"),
		textures_path("player/walk2.png"),
		textures_path("player/action1_weapon.png"),
		textures_path("player/action2_weapon.png"),
		textures_path("map/cracked_dirt.png"),

		textures_path("tutorials/move.png"),
		textures_path("tutorials/move_w.png"),
		textures_path("tutorials/move_a.png"),
		textures_path("tutorials/move_s.png"),
		textures_path("tutorials/move_d.png"),
		textures_path("tutorials/attack.png"),
		textures_path("tutorials/attack_animation.png"),
		textures_path("tutorials/plant.png"),
		textures_path("tutorials/plant_animation.png"),
		textures_path("tutorials/restart.png"),
		textures_path("tutorials/restart_animation.png"),
		textures_path("tutorials/dash.png"),
		textures_path("tutorials/dash_animation_1.png"),
		textures_path("tutorials/dash_animation_2.png"),
		textures_path("tutorials/change_seed.png"),
		textures_path("tutorials/change_seed_1.png"),
		textures_path("tutorials/change_seed_2.png"),
		textures_path("tutorials/change_seed_3.png"),
		textures_path("tutorials/change_seed_4.png"),
		textures_path("tutorials/change_seed_5.png"),
		textures_path("tutorials/change_seed_6.png"),
		textures_path("tutorials/change_seed_7.png"),
		textures_path("tutorials/change_seed_8.png"),
		textures_path("tutorials/arrow.png"),

		textures_path("ui/toolbar.png"),
		textures_path("attacks/Slash3_color1_frame1.png"),
		textures_path("attacks/Slash3_color1_frame2.png"),
		textures_path("attacks/Slash3_color1_frame3.png"),
		textures_path("attacks/Slash3_color1_frame4.png"),
		textures_path("attacks/Slash3_color1_frame5.png"),
		textures_path("attacks/Slash3_color1_frame6.png"),
		textures_path("attacks/Slash3_color1_frame7.png"),
		textures_path("attacks/Slash3_color1_frame8.png"),
		textures_path("attacks/Slash3_color1_frame9.png"),
		textures_path("towers/gold_bubble.png"),
		textures_path("towers/orange_bubble.png"),
		textures_path("towers/red_bubble.png"),
		textures_path("gameover/gameover.png"),
		textures_path("map/grass_background.png"),
		textures_path("map/flower_1.png"),
		textures_path("map/stone_1.png"),
		textures_path("map/grass_decoration_1.png"),
		textures_path("map/tree_1.png"),
		textures_path("map/farmland_1.png"),
		textures_path("map/stone_8.png"),
		textures_path("map/tree_2.png"),
		textures_path("enemies/SkeletonArcher-Idle1.png"),
		textures_path("enemies/SkeletonArcher-Idle2.png"),
		textures_path("enemies/SkeletonArcher-Idle3.png"),
		textures_path("enemies/SkeletonArcher-Idle4.png"),
		textures_path("enemies/SkeletonArcher-Idle5.png"),
		textures_path("enemies/SkeletonArcher-Idle6.png"),
		textures_path("enemies/SkeletonArcher-Walk1.png"),
		textures_path("enemies/SkeletonArcher-Walk2.png"),
		textures_path("enemies/SkeletonArcher-Walk3.png"),
		textures_path("enemies/SkeletonArcher-Walk4.png"),
		textures_path("enemies/SkeletonArcher-Walk5.png"),
		textures_path("enemies/SkeletonArcher-Walk6.png"),
		textures_path("enemies/SkeletonArcher-Walk7.png"),
		textures_path("enemies/SkeletonArcher-Walk8.png"),
		textures_path("enemies/SkeletonArcher-Attack1.png"),
		textures_path("enemies/SkeletonArcher-Attack2.png"),
		textures_path("enemies/SkeletonArcher-Attack3.png"),
		textures_path("enemies/SkeletonArcher-Attack4.png"),
		textures_path("enemies/SkeletonArcher-Attack5.png"),
		textures_path("enemies/SkeletonArcher-Attack6.png"),
		textures_path("enemies/SkeletonArcher-Attack7.png"),
		textures_path("enemies/SkeletonArcher-Attack8.png"),
		textures_path("enemies/SkeletonArcher-Attack9.png"),
		textures_path("enemies/Arrow.png"),
		textures_path("enemies/Orc-Walk1.png"),
		textures_path("enemies/Orc-Walk2.png"),
		textures_path("enemies/Orc-Walk3.png"),
		textures_path("enemies/Orc-Walk4.png"),
		textures_path("enemies/Orc-Walk5.png"),
		textures_path("enemies/Orc-Walk6.png"),
		textures_path("enemies/Orc-Walk7.png"),
		textures_path("enemies/Orc-Walk8.png"),

		textures_path("enemies/orc_elite/OrcElite_Walk_1.png"),
		textures_path("enemies/orc_elite/OrcElite_Walk_2.png"),
		textures_path("enemies/orc_elite/OrcElite_Walk_3.png"),
		textures_path("enemies/orc_elite/OrcElite_Walk_4.png"),
		textures_path("enemies/orc_elite/OrcElite_Walk_5.png"),
		textures_path("enemies/orc_elite/OrcElite_Walk_6.png"),
		textures_path("enemies/orc_elite/OrcElite_Walk_7.png"),
		textures_path("enemies/orc_elite/OrcElite_Walk_8.png"),

		textures_path("enemies/skeleton/Skeleton_Walk_1.png"),
		textures_path("enemies/skeleton/Skeleton_Walk_2.png"),
		textures_path("enemies/skeleton/Skeleton_Walk_3.png"),
		textures_path("enemies/skeleton/Skeleton_Walk_4.png"),
		textures_path("enemies/skeleton/Skeleton_Walk_5.png"),
		textures_path("enemies/skeleton/Skeleton_Walk_6.png"),
		textures_path("enemies/skeleton/Skeleton_Walk_7.png"),
		textures_path("enemies/skeleton/Skeleton_Walk_8.png"),

		textures_path("enemies/slime/Slime_Walk_1.png"),
		textures_path("enemies/slime/Slime_Walk_2.png"),
		textures_path("enemies/slime/Slime_Walk_3.png"),
		textures_path("enemies/slime/Slime_Walk_4.png"),
		textures_path("enemies/slime/Slime_Walk_5.png"),
		textures_path("enemies/slime/Slime_Walk_6.png"),

		textures_path("enemies/werebear/Werebear_Walk_1.png"),
		textures_path("enemies/werebear/Werebear_Walk_2.png"),
		textures_path("enemies/werebear/Werebear_Walk_3.png"),
		textures_path("enemies/werebear/Werebear_Walk_4.png"),
		textures_path("enemies/werebear/Werebear_Walk_5.png"),
		textures_path("enemies/werebear/Werebear_Walk_6.png"),
		textures_path("enemies/werebear/Werebear_Walk_7.png"),
		textures_path("enemies/werebear/Werebear_Walk_8.png"),

		textures_path("enemies/werewolf/Werewolf_Walk_1.png"),
		textures_path("enemies/werewolf/Werewolf_Walk_2.png"),
		textures_path("enemies/werewolf/Werewolf_Walk_3.png"),
		textures_path("enemies/werewolf/Werewolf_Walk_4.png"),
		textures_path("enemies/werewolf/Werewolf_Walk_5.png"),
		textures_path("enemies/werewolf/Werewolf_Walk_6.png"),
		textures_path("enemies/werewolf/Werewolf_Walk_7.png"),
		textures_path("enemies/werewolf/Werewolf_Walk_8.png"),

		textures_path("enemies/orc_rider/Orcrider_Idle1.png"),
		textures_path("enemies/orc_rider/Orcrider_Idle2.png"),
		textures_path("enemies/orc_rider/Orcrider_Idle3.png"),
		textures_path("enemies/orc_rider/Orcrider_Idle4.png"),
		textures_path("enemies/orc_rider/Orcrider_Idle5.png"),
		textures_path("enemies/orc_rider/Orcrider_Idle6.png"),

		textures_path("enemies/orc_rider/Orcrider_Walk1.png"),
		textures_path("enemies/orc_rider/Orcrider_Walk2.png"),
		textures_path("enemies/orc_rider/Orcrider_Walk3.png"),
		textures_path("enemies/orc_rider/Orcrider_Walk4.png"),
		textures_path("enemies/orc_rider/Orcrider_Walk5.png"),
		textures_path("enemies/orc_rider/Orcrider_Walk6.png"),
		textures_path("enemies/orc_rider/Orcrider_Walk7.png"),
		textures_path("enemies/orc_rider/Orcrider_Walk8.png"),

		textures_path("enemies/orc_rider/Orcrider_Hunt1.png"),
		textures_path("enemies/orc_rider/Orcrider_Hunt2.png"),
		textures_path("enemies/orc_rider/Orcrider_Hunt3.png"),
		textures_path("enemies/orc_rider/Orcrider_Hunt4.png"),

		textures_path("player/playerIdle(100)/Player-Idle1.png"),
		textures_path("player/playerIdle(100)/Player-Idle2.png"),
		textures_path("player/playerIdle(100)/Player-Idle3.png"),
		textures_path("player/playerIdle(100)/Player-Idle4.png"),
		textures_path("player/playerIdle(100)/Player-Idle5.png"),
		textures_path("player/playerIdle(100)/Player-Idle6.png"),

		textures_path("player/playerWalk(100)/Player-Walk1.png"),
		textures_path("player/playerWalk(100)/Player-Walk2.png"),
		textures_path("player/playerWalk(100)/Player-Walk3.png"),
		textures_path("player/playerWalk(100)/Player-Walk4.png"),
		textures_path("player/playerWalk(100)/Player-Walk5.png"),
		textures_path("player/playerWalk(100)/Player-Walk6.png"),
		textures_path("player/playerWalk(100)/Player-Walk7.png"),
		textures_path("player/playerWalk(100)/Player-Walk8.png"),

		textures_path("player/playerAttack(100)/Player-Attack1.png"),
		textures_path("player/playerAttack(100)/Player-Attack2.png"),
		textures_path("player/playerAttack(100)/Player-Attack3.png"),
		textures_path("player/playerAttack(100)/Player-Attack4.png"),
		textures_path("player/playerAttack(100)/Player-Attack5.png"),
		textures_path("player/playerAttack(100)/Player-Attack6.png"),
		textures_path("towers/plant2/idle/Plant2_idle_f.png"),
		textures_path("towers/plant2/idle/Plant2_idle_b.png"),
		textures_path("towers/plant2/idle/Plant2_idle_s.png"),
		textures_path("towers/plant2/attack/Plant2_attack_f1.png"),
		textures_path("towers/plant2/attack/Plant2_attack_f2.png"),
		textures_path("towers/plant2/attack/Plant2_attack_b1.png"),
		textures_path("towers/plant2/attack/Plant2_attack_b2.png"),
		textures_path("towers/plant2/attack/Plant2_attack_s1.png"),
		textures_path("towers/plant2/attack/Plant2_attack_s2.png"),
    
    	textures_path("plants/plant_1/Plant1_Idle_1.png"),
		textures_path("plants/plant_1/Plant1_Idle_2.png"),
		textures_path("plants/plant_1/Plant1_Idle_3.png"),
		textures_path("plants/plant_1/Plant1_Idle_4.png"),
		textures_path("plants/plant_1/Plant1_Attack_1.png"),
		textures_path("plants/plant_1/Plant1_Attack_2.png"),
		textures_path("plants/plant_1/Plant1_Attack_3.png"),
		textures_path("plants/plant_1/Plant1_Attack_4.png"),
		textures_path("plants/plant_1/Plant1_Attack_5.png"),
		textures_path("plants/plant_1/Plant1_Attack_6.png"),
		textures_path("plants/plant_1/Plant1_Attack_7.png"),
		textures_path("plants/plant_2/Plant2_Idle_1.png"),
		textures_path("plants/plant_2/Plant2_Idle_2.png"),
		textures_path("plants/plant_2/Plant2_Idle_3.png"),
		textures_path("plants/plant_2/Plant2_Idle_4.png"),
		textures_path("plants/plant_2/Plant2_Attack_1.png"),
		textures_path("plants/plant_2/Plant2_Attack_2.png"),
		textures_path("plants/plant_2/Plant2_Attack_3.png"),
		textures_path("plants/plant_2/Plant2_Attack_4.png"),
		textures_path("plants/plant_2/Plant2_Attack_5.png"),
		textures_path("plants/plant_2/Plant2_Attack_6.png"),
		textures_path("plants/plant_2/Plant2_Attack_7.png"),
		textures_path("plants/plant_3/Plant3_Idle_1.png"),
		textures_path("plants/plant_3/Plant3_Idle_2.png"),
		textures_path("plants/plant_3/Plant3_Idle_3.png"),
		textures_path("plants/plant_3/Plant3_Idle_4.png"),
		textures_path("plants/plant_3/Plant3_Attack_1.png"),
		textures_path("plants/plant_3/Plant3_Attack_2.png"),
		textures_path("plants/plant_3/Plant3_Attack_3.png"),
		textures_path("plants/plant_3/Plant3_Attack_4.png"),
		textures_path("plants/plant_3/Plant3_Attack_5.png"),
		textures_path("plants/plant_3/Plant3_Attack_6.png"),
		textures_path("plants/plant_3/Plant3_Attack_7.png"),

		textures_path("plants/plant_1_purple/idle/00.png"),
		textures_path("plants/plant_1_purple/idle/01.png"),
		textures_path("plants/plant_1_purple/idle/02.png"),
		textures_path("plants/plant_1_purple/idle/03.png"),
		textures_path("plants/plant_1_purple/attack/00.png"),
		textures_path("plants/plant_1_purple/attack/01.png"),
		textures_path("plants/plant_1_purple/attack/02.png"),
		textures_path("plants/plant_1_purple/attack/03.png"),
		textures_path("plants/plant_1_purple/attack/04.png"),
		textures_path("plants/plant_1_purple/attack/05.png"),
		textures_path("plants/plant_1_purple/attack/06.png"),
		textures_path("plants/plant_1_yellow/idle/00.png"),
		textures_path("plants/plant_1_yellow/idle/01.png"),
		textures_path("plants/plant_1_yellow/idle/02.png"),
		textures_path("plants/plant_1_yellow/idle/03.png"),
		textures_path("plants/plant_1_yellow/attack/00.png"),
		textures_path("plants/plant_1_yellow/attack/01.png"),
		textures_path("plants/plant_1_yellow/attack/02.png"),
		textures_path("plants/plant_1_yellow/attack/03.png"),
		textures_path("plants/plant_1_yellow/attack/04.png"),
		textures_path("plants/plant_1_yellow/attack/05.png"),
		textures_path("plants/plant_1_yellow/attack/06.png"),
		textures_path("plants/plant_2_purple/idle/00.png"),
		textures_path("plants/plant_2_purple/idle/01.png"),
		textures_path("plants/plant_2_purple/idle/02.png"),
		textures_path("plants/plant_2_purple/idle/03.png"),
		textures_path("plants/plant_2_purple/attack/00.png"),
		textures_path("plants/plant_2_purple/attack/01.png"),
		textures_path("plants/plant_2_purple/attack/02.png"),
		textures_path("plants/plant_2_purple/attack/03.png"),
		textures_path("plants/plant_2_purple/attack/04.png"),
		textures_path("plants/plant_2_purple/attack/05.png"),
		textures_path("plants/plant_2_purple/attack/06.png"),
		textures_path("plants/plant_2_cyan/idle/00.png"),
		textures_path("plants/plant_2_cyan/idle/01.png"),
		textures_path("plants/plant_2_cyan/idle/02.png"),
		textures_path("plants/plant_2_cyan/idle/03.png"),
		textures_path("plants/plant_2_cyan/attack/00.png"),
		textures_path("plants/plant_2_cyan/attack/01.png"),
		textures_path("plants/plant_2_cyan/attack/02.png"),
		textures_path("plants/plant_2_cyan/attack/03.png"),
		textures_path("plants/plant_2_cyan/attack/04.png"),
		textures_path("plants/plant_2_cyan/attack/05.png"),
		textures_path("plants/plant_2_cyan/attack/06.png"),
		textures_path("plants/plant_3_blue/idle/00.png"),
		textures_path("plants/plant_3_blue/idle/01.png"),
		textures_path("plants/plant_3_blue/idle/02.png"),
		textures_path("plants/plant_3_blue/idle/03.png"),
		textures_path("plants/plant_3_blue/attack/00.png"),
		textures_path("plants/plant_3_blue/attack/01.png"),
		textures_path("plants/plant_3_blue/attack/02.png"),
		textures_path("plants/plant_3_blue/attack/03.png"),
		textures_path("plants/plant_3_blue/attack/04.png"),
		textures_path("plants/plant_3_blue/attack/05.png"),
		textures_path("plants/plant_3_blue/attack/06.png"),
    
		textures_path("ui/new_game.png"),
		textures_path("ui/load_game.png"),
		textures_path("ui/tutorial.png"),
		textures_path("ui/quit.png"),
		textures_path("ui/pause.png"),
		textures_path("ui/save.png"),
		textures_path("seeds/seed_0.png"),
		textures_path("seeds/seed_3.png"),
		textures_path("seeds/seed_4.png"),
		textures_path("seeds/seed_1.png"),
		textures_path("seeds/seed_2.png"),
		textures_path("seeds/seed_5.png"),
		textures_path("seeds/seed_6.png"),
		textures_path("seeds/seed_7.png"),
		textures_path("ui/return.png"),
		textures_path("ui/resume.png"),
		textures_path("ui/background.png"),
		textures_path("ui/pause_panel.png"),
		textures_path("ui/selected.png"),
		textures_path("particles/particle1.png"),
		textures_path("cut_scenes/night_bg.png"),
		textures_path("cut_scenes/day_bg.png"),
		textures_path("cut_scenes/chicken.png"),
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("egg"),
		shader_path("chicken"),
		shader_path("textured"),
		shader_path("ui"),
		shader_path("vignette"),
		shader_path("zombie"),
		shader_path("player"),
		shader_path("particle"),
	};

	// fonts
	struct Character
	{
		unsigned int TextureID; // ID handle of the glyph texture
		glm::ivec2 Size;		// Size of glyph
		glm::ivec2 Bearing;		// Offset from baseline to left/top of glyph
		unsigned int Advance;	// Offset to advance to next glyph
		char character;
	};

	// Font
	std::map<char, Character> m_ftCharacters;
	GLuint m_font_shaderProgram;
	GLuint m_font_VAO;
	GLuint m_font_VBO;
	glm::mat4 trans = glm::mat4(1.0f);

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow *window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();

	Mesh &getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();

	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the vignette shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void step_and_draw(float elapsed_ms);

	mat3 createProjectionMatrix();
	mat3 createProjectionMatrix_splash();
	
	Entity get_screen_state_entity() { return screen_state_entity; }

	bool fontInit(const std::string &font_filename, unsigned int font_default_size);
	void renderText(std::string text, float x, float y, float scale, const glm::vec3 &color, const glm::mat4 &trans);

private:
	// Internal drawing functions for each entity type
	void drawGridLine(Entity entity, const mat3 &projection);
	void drawTexturedMesh(Entity entity, const mat3 &projection);
	void drawToScreen();
	void drawUI();

	// Window handle
	GLFWwindow *window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
	GLuint vao;

	// For instanced particle rendering
	GLuint particle_instance_VBO = 0;
	const int MAX_PARTICLES = 2000;

	// New method to render particles with instancing
	void drawParticlesInstanced(const mat3 &projection);
};

bool loadEffectFromFile(
	const std::string &vs_path, const std::string &fs_path, GLuint &out_program);
