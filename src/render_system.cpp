
#include <SDL.h>
#include <glm/trigonometric.hpp>
#include <iostream>

// internal
#include "render_system.hpp"
#include "tinyECS/registry.hpp"
#include "world_system.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <sstream>

void RenderSystem::drawGridLine(Entity entity,
								const mat3 &projection)
{

	GridLine &gridLine = registry.gridLines.get(entity);

	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(gridLine.start_pos);
	transform.scale(gridLine.end_pos);

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	gl_has_errors();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	if (render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		gl_has_errors();

		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &projection)
{
	Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT

	// Get visual scale if available, otherwise use {1,1}
	vec2 visualScale = {1.0f, 1.0f};
	if (registry.visualScales.has(entity))
	{
		visualScale = registry.visualScales.get(entity).scale;
	}
	Transform transform;
	transform.translate(motion.position);
	transform.scale(motion.scale * visualScale);
	transform.rotate(radians(motion.angle));

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);

	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// // Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	gl_has_errors();

	// texture-mapped entities - use data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::ZOMBIE)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// handle alpha
		float alpha = 1.0f;
		if (registry.deathAnimations.has(entity))
		{
			alpha = registry.deathAnimations.get(entity).alpha;
		}
		GLint alpha_loc = glGetUniformLocation(program, "alpha");
		glUniform1f(alpha_loc, alpha);
		gl_has_errors();

		// handle hit effect
		bool zombie_is_hit = registry.hitEffects.has(entity);
		GLint hit_loc = glGetUniformLocation(program, "zombie_is_hit");
		glUniform1i(hit_loc, zombie_is_hit);
		gl_has_errors();

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::PLAYER)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(vec3)); // note the stride to skip the preceeding vertex position

		// handle hit effect
		bool player_is_hit = registry.hitEffects.has(entity);
		GLint hit_loc = glGetUniformLocation(program, "player_is_hit");
		glUniform1i(hit_loc, player_is_hit);
		gl_has_errors();

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::CHICKEN || render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::PARTICLE)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(vec3));
		gl_has_errors();

		// Pass color as vec4 including alpha
		vec4 particleColor = {1.0f, 1.0f, 1.0f, 1.0f};
		if (registry.particles.has(entity))
		{
			Particle &particle = registry.particles.get(entity);
			particleColor = particle.Color;
		}

		GLint color_loc = glGetUniformLocation(program, "color");
		glUniform4fv(color_loc, 1, &particleColor[0]);
		gl_has_errors();

		// Set particle type based on the particle's generator type
		int particleType = 0; // Default type for regular particles
		if (registry.particles.has(entity))
		{
			// Find the generator that created this particle
			for (Entity gen_entity : registry.particleGenerators.entities)
			{
				const ParticleGenerator &generator = registry.particleGenerators.get(gen_entity);

				// Check if this particle is in this generator's list
				bool found = false;
				for (unsigned int i = 0; i < generator.particles.size(); i++)
				{
					if (generator.particles[i].id() == entity.id())
					{
						found = true;
						// Set type based on generator type
						if (generator.type == "electricity_line")
							particleType = 1;
						break;
					}
				}

				if (found)
					break;
			}
		}

		// Set the uniform for the particle type
		GLint particleType_loc = glGetUniformLocation(program, "particleType");
		if (particleType_loc >= 0) // Make sure the uniform exists
		{
			glUniform1i(particleType_loc, particleType);
			gl_has_errors();
		}
		else
		{
			// Debug output if the uniform isn't found
			std::cout << "Warning: particleType uniform not found in shader" << std::endl;
		}

		// Pass life ratio for visual effects
		float lifeRatio = 1.0f;
		if (registry.particles.has(entity))
		{
			Particle &particle = registry.particles.get(entity);
			lifeRatio = particle.Life / particle.MaxLife;
		}

		GLint life_loc = glGetUniformLocation(program, "life_ratio");
		if (life_loc >= 0)
		{
			glUniform1f(life_loc, lifeRatio);
			gl_has_errors();
		}

		// Enabling and binding texture
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];
		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

// first draw to an intermediate texture,
// apply the "UI" texture, when requested
// then draw the intermediate texture
void RenderSystem::drawToScreen()
{
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();

	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	// if (!WorldSystem::game_is_over) {
	if (1) {
		// add the "UI" effect
		const GLuint ui_program = effects[(GLuint)EFFECT_ASSET_ID::UI];
		glUseProgram(ui_program);
		// set clock
		GLuint hp_uloc = glGetUniformLocation(ui_program, "hp_percentage");
		GLuint exp_uloc = glGetUniformLocation(ui_program, "exp_percentage");
		GLuint game_continues_uloc = glGetUniformLocation(ui_program, "game_over");
		GLuint dark_uloc = glGetUniformLocation(ui_program, "darken_factor");

		glUniform1f(dark_uloc, screen.darken_screen_factor);
		glUniform1f(game_continues_uloc, screen.game_over);
		glUniform1f(hp_uloc, (WorldSystem::get_game_screen() == GAME_SCREEN_ID::SPLASH || WorldSystem::get_game_screen() == GAME_SCREEN_ID::CG || WorldSystem::get_game_screen() == GAME_SCREEN_ID::PAUSE || WorldSystem::get_game_screen() == GAME_SCREEN_ID::LEVEL_UP|| WorldSystem::get_game_screen() == GAME_SCREEN_ID::GAME_OVER)? 0 : screen.hp_percentage);
		glUniform1f(exp_uloc, (WorldSystem::get_game_screen() == GAME_SCREEN_ID::SPLASH || WorldSystem::get_game_screen() == GAME_SCREEN_ID::CG || WorldSystem::get_game_screen() == GAME_SCREEN_ID::PAUSE || WorldSystem::get_game_screen() == GAME_SCREEN_ID::LEVEL_UP|| WorldSystem::get_game_screen() == GAME_SCREEN_ID::GAME_OVER) ? 0 : screen.exp_percentage);
		gl_has_errors();

		// Set the vertex position and vertex texture coordinates (both stored in the
		// same VBO)
		GLint in_position_loc = glGetAttribLocation(ui_program, "in_position");
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
		gl_has_errors();
	}
	else
	{
		// add the "gameover" effect
		const GLuint vignette_program = effects[(GLuint)EFFECT_ASSET_ID::VIGNETTE];
		glUseProgram(vignette_program);
		GLuint time_uloc1 = glGetUniformLocation(vignette_program, "time");
		GLuint dead_timer_uloc1 = glGetUniformLocation(vignette_program, "darken_screen_factor");
		GLuint game_continues_uloc1 = glGetUniformLocation(vignette_program, "game_over");
		GLuint tex_offset_uloc = glGetUniformLocation(vignette_program, "tex_offset");

		GLint in_position_loc1 = glGetAttribLocation(vignette_program, "in_position");
		// std::cout<<screen.lerp_timer/2000<<std::endl;
		glUniform1f(time_uloc1, screen.lerp_timer);
		glUniform1f(game_continues_uloc1, screen.game_over);
		glUniform2f(tex_offset_uloc, registry.motions.get(registry.players.entities[0]).position.x, registry.motions.get(registry.players.entities[0]).position.y);
		glEnableVertexAttribArray(in_position_loc1);
		glVertexAttribPointer(in_position_loc1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
		gl_has_errors();
	}

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();

	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer

	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::step_and_draw(float elapsed_ms)
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	gl_has_errors();
	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// clear backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);

	// grass background
	glClearColor(GRASS_COLOR.x, GRASS_COLOR.y, GRASS_COLOR.z, 1.0f);
	// glClearColor(0.016f, 0.098f, 0.18f, 1.0f);

	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	int cutscene = registry.screenStates.components[0].cutscene;
	mat3 projection_2D = (WorldSystem::get_game_screen() == GAME_SCREEN_ID::SPLASH || WorldSystem::get_game_screen() == GAME_SCREEN_ID::CG || WorldSystem::get_game_screen() == GAME_SCREEN_ID::GAME_OVER) ? createProjectionMatrix_splash() : createProjectionMatrix();

	if (WorldSystem::get_game_screen() == GAME_SCREEN_ID::SPLASH || WorldSystem::get_game_screen() == GAME_SCREEN_ID::CG || WorldSystem::get_game_screen() == GAME_SCREEN_ID::GAME_OVER)
	{
		for (Entity entity : registry.cgs.entities)
		{
			if (registry.renderRequests.has(entity)) drawTexturedMesh(entity, projection_2D);
		}

		if (WorldSystem::get_game_screen() == GAME_SCREEN_ID::SPLASH) {
			renderText("Farmer Defense", WINDOW_WIDTH_PX / 3, WINDOW_HEIGHT_PX - 100, OS_RES, {0, 0, 0}, trans);
		} else if (WorldSystem::get_game_screen() == GAME_SCREEN_ID::CG) {
			int cg_idx = registry.screenStates.components[0].cg_index;
			int cutscene = registry.screenStates.components[0].cutscene;
			if (cutscene == 1)
			{
				if (cg_idx == 0)
					renderText("After years on the battlefield, you hung up your sword and returned home...", 10, WINDOW_HEIGHT_PX - 100, 0.6 * OS_RES, {1, 1, 1}, trans);
				if (cg_idx == 1)
					renderText("You inherited the farm from your parents, and became a farmer...", 10, WINDOW_HEIGHT_PX - 100, 0.6 * OS_RES, {1, 1, 1}, trans);
				if (cg_idx == 2)
					renderText("But one night, a meteor carrying a strange virus struck the city...", 10, WINDOW_HEIGHT_PX - 100, 0.6 * OS_RES, {1, 1, 1}, trans);
				if (cg_idx == 3)
					renderText("*explosion from a distant space", 10, WINDOW_HEIGHT_PX - 100, 0.6, {1, 1, 1}, trans);
				if (cg_idx == 4)
				{
					renderText("You heard the sound, walk up to the window, peered out but everything seems...", 10, WINDOW_HEIGHT_PX - 100, 0.6 * OS_RES, {1, 1, 1}, trans);
					renderText("peaceful and normal...", 10, WINDOW_HEIGHT_PX - 150, 0.6, {1, 1, 1}, trans);
				}
				if (cg_idx == 5)
					renderText("You went back to sleep, unaware that your world had already changed...", 10, WINDOW_HEIGHT_PX - 100, 0.6 * OS_RES, {1, 1, 1}, trans);
				if (cg_idx == 6)
					renderText("*The next day", 10, WINDOW_HEIGHT_PX - 100, 0.6 * OS_RES, {1, 1, 1}, trans);
				if (cg_idx == 8)
					renderText("Uh... Hello?", 60, 350, 0.6 * OS_RES, {1, 1, 1}, trans);
				if (cg_idx == 9)
					renderText("*Growl", WINDOW_WIDTH_PX - 400, 350, 0.6 * OS_RES, {1, 1, 1}, trans);
				if (cg_idx == 10)
					renderText("Uh oh...", 60, 350, 0.6 * OS_RES, {1, 1, 1}, trans);
			}
			else if (cutscene == 2)
			{
				// second scene, plant grow? yes plant grow!
				if (cg_idx == 1)
					renderText("Bro I thought I would get some carrots, not you?!", 60, 350, 0.6 * OS_RES, {1, 1, 1}, trans);
				if (cg_idx == 2)
					renderText("I am here to only help!", WINDOW_WIDTH_PX - 500, 350, 0.6 * OS_RES, {1, 1, 1}, trans);
				if (cg_idx == 3)
					renderText("WTH you can talk??", 60, 350, 0.6 * OS_RES, {1, 1, 1}, trans);
				if (cg_idx == 4)
				{
					renderText("The zombies can hold weapons,", WINDOW_WIDTH_PX - 700, 450, 0.6 * OS_RES, {1, 1, 1}, trans);
					renderText("so WHY NOT", WINDOW_WIDTH_PX - 700, 350, 0.6 * OS_RES, {1, 1, 1}, trans);
				}
				if (cg_idx == 5)
					renderText("Alright...", 60, 350, 0.6 * OS_RES, {1, 1, 1}, trans);
			}
			else if (cutscene == 3)
			{
				if (cg_idx == 1)
					renderText("Summon the chicken!", WINDOW_WIDTH_PX - 700, 450, 0.6 * OS_RES, {1, 1, 1}, trans);
				if (cg_idx == 2)
					renderText("How should I do that", 60, 350, 0.6 * OS_RES, {1, 1, 1}, trans);
				if (cg_idx == 3)
					renderText("\'C\' for the chicken!", WINDOW_WIDTH_PX - 700, 450, 0.6 * OS_RES, {1, 1, 1}, trans);
				if (cg_idx == 4)
					renderText("Alright...", 60, 350, 0.6 * OS_RES, {1, 1, 1}, trans);
			}
		} else if (WorldSystem::get_game_screen() == GAME_SCREEN_ID::GAME_OVER) {
			for (Entity text_entity : registry.texts.entities) {
				renderText(registry.texts.get(text_entity).text, registry.texts.get(text_entity).pos.x, registry.texts.get(text_entity).pos.y, registry.texts.get(text_entity).size, registry.texts.get(text_entity).color, trans);
			}

			renderText("GAME OVER", WINDOW_WIDTH_PX * 0.25, WINDOW_HEIGHT_PX * 0.8, 2.0f, glm::vec3(0.0f, 0.0f, 0.0f), trans);
		}

		drawToScreen();
	} else {
		// draw all entities with a render request to the frame buffer
		for (Entity entity : registry.renderRequests.entities)
		{
			// filter to entities that have a motion component
			if (registry.motions.has(entity) && registry.renderRequests.get(entity).used_geometry != GEOMETRY_BUFFER_ID::DEBUG_LINE && !registry.moveWithCameras.has(entity))
			{
				// Note, its not very efficient to access elements indirectly via the entity
				// albeit iterating through all Sprites in sequence. A good point to optimize
				if (WorldSystem::get_game_screen() == GAME_SCREEN_ID::TUTORIAL)
				{
					if (registry.mapTiles.has(entity))
					{
						if (registry.tutorialTiles.has(entity))
						{
							drawTexturedMesh(entity, projection_2D);
						}
					}
					else
					{
						drawTexturedMesh(entity, projection_2D);
					}
				}
				else if (!registry.tutorialSigns.has(entity) && !registry.tutorialTiles.has(entity))
				{
					drawTexturedMesh(entity, projection_2D);
				}
			}
			// draw grid lines separately, as they do not have motion but need to be rendered
			else if (registry.gridLines.has(entity))
			{
				drawGridLine(entity, projection_2D);
			}
		}
		drawParticlesInstanced(projection_2D);

		// individually draw player, toolbar, inventory seeds, pause button; will render on top of all the motion sprites
		if (!WorldSystem::game_is_over && WorldSystem::get_game_screen() != GAME_SCREEN_ID::PAUSE && WorldSystem::get_game_screen() != GAME_SCREEN_ID::LEVEL_UP)
		{
			for (Entity entity : registry.moveWithCameras.entities)
			{
				if (registry.renderRequests.has(entity))
					drawTexturedMesh(entity, projection_2D);
			}
		}


		if(WorldSystem::get_game_screen() != GAME_SCREEN_ID::LEVEL_UP && WorldSystem::get_game_screen() != GAME_SCREEN_ID::PAUSE) {
			renderText("HP", WINDOW_WIDTH_PX * 0.625, WINDOW_HEIGHT_PX * 0.925, 0.75, {1, 1, 1}, trans);
			renderText("EXP", WINDOW_WIDTH_PX * 0.625, WINDOW_HEIGHT_PX * 0.85, 0.75, {1, 1, 1}, trans);

			for (Entity seed_entity : registry.seeds.entities)
			{
				if (registry.motions.has(seed_entity) && registry.moveWithCameras.has(seed_entity))
				{
					if (registry.inventorys.size() != 0) {
						int seed_type = registry.seeds.get(seed_entity).type;
						int current_seed_count = registry.inventorys.components[0].seedCount[seed_type];
						vec2 seed_pos = registry.motions.get(seed_entity).position;
						#if __APPLE__
						renderText(std::to_string(current_seed_count), WINDOW_WIDTH_PX / 2 - TOOLBAR_WIDTH / 2 + TOOLBAR_HEIGHT * (seed_type * 0.95 + 0.9), 22.5, 0.25, {0.7, 0.25, 0.25}, trans);
						#else
						renderText(std::to_string(current_seed_count), WINDOW_WIDTH_PX / 2 - 550 / 2 + 550 / 8 * seed_type + 5, 25, 0.25, {0.7, 0.25, 0.25}, trans);
						#endif
					}
				}
			}
			for (Entity text_entity : registry.texts.entities)
			{
				renderText(registry.texts.get(text_entity).text, registry.texts.get(text_entity).pos.x, registry.texts.get(text_entity).pos.y, registry.texts.get(text_entity).size, registry.texts.get(text_entity).color, trans);
			}
			// Render the FPS counter
			float current_fps = (1 / (elapsed_ms / 1000));
			renderText("FPS: " + std::to_string(current_fps), WINDOW_WIDTH_PX * 0.05, WINDOW_HEIGHT_PX * 0.925, 0.3, {0, 1, 1}, trans);

			// Render the number of enemies on screen
			renderText("Enemy count: " + std::to_string(registry.enemies.size()), WINDOW_WIDTH_PX * 0.05, WINDOW_HEIGHT_PX * 0.875, 0.3, {0, 1, 1}, trans);

			// Render the number of plants on screen
			int total_seed_count = 0;
			for (Entity entity : registry.seeds.entities) {
				if (!registry.moveWithCameras.has(entity)) total_seed_count++;
			}
			renderText("Plant count: " + std::to_string(total_seed_count + registry.towers.size()), WINDOW_WIDTH_PX * 0.05, WINDOW_HEIGHT_PX * 0.825, 0.3, {0, 1, 1}, trans);

		}


		if(WorldSystem::get_game_screen() == GAME_SCREEN_ID::LEVEL_UP) {
			renderText("LEVEL UP!", WINDOW_WIDTH_PX /2-100*OS_RES, WINDOW_HEIGHT_PX - 175*OS_RES, OS_RES*0.7, {1,1,1}, trans);
			renderText("Choose one of 4 options", WINDOW_WIDTH_PX / 3, WINDOW_HEIGHT_PX - 300, OS_RES*0.7, {1, 1, 1}, trans);
		}
		//  draw framebuffer to screen
		//  adding "UI" effect when applied
		drawToScreen();
	}

	// // draw all particles
	// for (Entity entity : registry.particles.entities)
	// {
	// 	if (registry.renderRequests.has(entity))
	// 	{
	// 		drawTexturedMesh(entity, projection_2D);
	// 	}
	// }

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix_splash()
{
	// fake projection matrix, scaled to window coordinates
	float left = 0.f;
	float top = 0.f;
	float right = (float)WINDOW_WIDTH_PX;
	float bottom = (float)WINDOW_HEIGHT_PX;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);

	return {
		{sx, 0.f, 0.f},
		{0.f, sy, 0.f},
		{tx, ty, 1.f}};
}

// mat3 RenderSystem::createProjectionMatrix() {
//     auto& screen = registry.screenStates.get(screen_state_entity);

//     float left = 0.f + screen.shake_offset.x;
//     float top = 0.f + screen.shake_offset.y;
//     float right = (float)WINDOW_WIDTH_PX + screen.shake_offset.x;
//     float bottom = (float)WINDOW_HEIGHT_PX + screen.shake_offset.y;

//     float sx = 2.f / (right - left);
//     float sy = 2.f / (top - bottom);
//     float tx = -(right + left) / (right - left);
//     float ty = -(top + bottom) / (top - bottom);

//     return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
// }

mat3 RenderSystem::createProjectionMatrix()
{
	auto &screen = registry.screenStates.get(screen_state_entity);
	auto &camera = registry.cameras.get(registry.cameras.entities[0]);

	// Center camera on player, accounting for window size
	float left = camera.position.x - camera.camera_width / 2 + screen.shake_offset.x;
	float top = camera.position.y - camera.camera_height / 2 + screen.shake_offset.y;
	float right = camera.position.x + camera.camera_width / 2 + screen.shake_offset.x;
	float bottom = camera.position.y + camera.camera_height / 2 + screen.shake_offset.y;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);

	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

bool is_shader_error(unsigned int shader, std::string shader_name)
{

	GLint fs_compile_result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &fs_compile_result);
	if (fs_compile_result != GL_TRUE)
	{
		std::cerr << "ERROR: shader compiler error for shader: " << shader_name << std::endl;

		char errBuff[1024];
		int bufLen = 0;
		glGetShaderInfoLog(shader, 1024, &bufLen, errBuff);
		std::cout << "Shader error info: " << errBuff << std::endl;
		assert(bufLen == 0);
		return true;
	}
	else
	{
		std::cout << "No error with shader: " << shader_name << std::endl;
		return false;
	}
}

std::string readShaderFile(const std::string &filename)
{
	// std::cout << "Loading shader filename: " << filename << std::endl;

	std::ifstream ifs(filename);

	if (!ifs.good())
	{
		std::cerr << "ERROR: invalid filename loading shader from file: " << filename << std::endl;
		return "";
	}

	std::ostringstream oss;
	oss << ifs.rdbuf();
	// std::cout << oss.str() << std::endl;
	return oss.str();
}

bool RenderSystem::fontInit(const std::string &font_filename, unsigned int font_default_size)
{

	// read in our shader files
	std::string vertexShaderSource = readShaderFile(PROJECT_SOURCE_DIR + std::string("shaders/font.vs.glsl"));
	std::string fragmentShaderSource = readShaderFile(PROJECT_SOURCE_DIR + std::string("shaders/font.fs.glsl"));
	const char *vertexShaderSource_c = vertexShaderSource.c_str();
	const char *fragmentShaderSource_c = fragmentShaderSource.c_str();

	// enable blending or you will just get solid boxes instead of text
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// font buffer setup
	glGenVertexArrays(1, &m_font_VAO);
	glGenBuffers(1, &m_font_VBO);

	// font vertex shader
	unsigned int font_vertexShader;
	font_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(font_vertexShader, 1, &vertexShaderSource_c, NULL);
	glCompileShader(font_vertexShader);

	// font fragement shader
	unsigned int font_fragmentShader;
	font_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(font_fragmentShader, 1, &fragmentShaderSource_c, NULL);
	glCompileShader(font_fragmentShader);

	// font shader program
	m_font_shaderProgram = glCreateProgram();
	glAttachShader(m_font_shaderProgram, font_vertexShader);
	glAttachShader(m_font_shaderProgram, font_fragmentShader);
	glLinkProgram(m_font_shaderProgram);

	// apply orthographic projection matrix for font, i.e., screen space
	glUseProgram(m_font_shaderProgram);
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(WINDOW_WIDTH_PX), 0.0f, static_cast<float>(WINDOW_HEIGHT_PX));
	GLint project_location = glGetUniformLocation(m_font_shaderProgram, "projection");
	assert(project_location > -1);
	glUniformMatrix4fv(project_location, 1, GL_FALSE, glm::value_ptr(projection));

	// clean up shaders
	glDeleteShader(font_vertexShader);
	glDeleteShader(font_fragmentShader);

	// init FreeType fonts
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
		return false;
	}

	FT_Face face;
	if (FT_New_Face(ft, font_filename.c_str(), 0, &face))
	{
		std::cerr << "ERROR::FREETYPE: Failed to load font: " << font_filename << std::endl;
		return false;
	}

	// extract a default size
	FT_Set_Pixel_Sizes(face, 0, font_default_size);

	// disable byte-alignment restriction in OpenGL
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// load each of the chars - note only first 128 ASCII chars
	for (unsigned char c = (unsigned char)0; c < (unsigned char)128; c++)
	{
		// load character glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cerr << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}

		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// std::cout << "texture: " << c << " = " << texture << std::endl;

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer);

		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<unsigned int>(face->glyph->advance.x),
			(char)c};
		m_ftCharacters.insert(std::pair<char, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	// clean up
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// bind buffers
	glBindVertexArray(m_font_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

	// // release buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

void RenderSystem::renderText(std::string text, float x, float y, float scale, const glm::vec3 &color, const glm::mat4 &trans)
{
	// Activate shader
	glUseProgram(m_font_shaderProgram);

	// enable blending or you will just get solid boxes instead of text
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLint textColor_location = glGetUniformLocation(m_font_shaderProgram, "textColor");
	assert(textColor_location > -1);
	// std::cout << "textColor_location: " << textColor_location << std::endl;
	glUniform3f(textColor_location, color.x, color.y, color.z);

	auto transformLoc = glGetUniformLocation(m_font_shaderProgram, "transform");
	// std::cout << "transformLoc: " << transformLoc << std::endl;
	assert(transformLoc > -1);
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

	glBindVertexArray(m_font_VAO);

	// iterate through each character
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = m_ftCharacters[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		float vertices[6][4] = {
			{xpos, ypos + h, 0.0f, 0.0f},
			{xpos, ypos, 0.0f, 1.0f},
			{xpos + w, ypos, 1.0f, 1.0f},

			{xpos, ypos + h, 0.0f, 0.0f},
			{xpos + w, ypos, 1.0f, 1.0f},
			{xpos + w, ypos + h, 1.0f, 0.0f}};

		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// std::cout << "binding texture: " << ch.character << " = " << ch.TextureID << std::endl;

		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// advance to next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(vao);
	// glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderSystem::drawParticlesInstanced(const mat3 &projection)
{
	// Skip if no particles
	if (registry.particles.entities.empty())
		return;

	// Count active particles
	int particleCount = (int)registry.particles.size();
	if (particleCount == 0)
		return;

	// Check for instanced rendering support
	GLint major = 0, minor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	bool instancing_supported = (major > 3 || (major == 3 && minor >= 3));

	if (!instancing_supported)
	{
		// Fallback to regular rendering
		for (Entity entity : registry.particles.entities)
		{
			if (registry.renderRequests.has(entity))
			{
				drawTexturedMesh(entity, projection);
			}
		}
		return;
	}

	// Enable blending for particles
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Use particle shader
	GLuint program = effects[(GLuint)EFFECT_ASSET_ID::PARTICLE];
	glUseProgram(program);
	gl_has_errors();

	// Prepare matrices
	GLint proj_loc = glGetUniformLocation(program, "projection");
	glUniformMatrix3fv(proj_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	// Bind default particle texture
	GLuint texture_id = texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::PARTICLE];
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	gl_has_errors();

	// Bind mesh data
	const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];
	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SPRITE];

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Get proper attribute locations from shader
	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	GLint instance_pos_scale_loc = glGetAttribLocation(program, "instance_pos_scale");
	GLint instance_color_loc = glGetAttribLocation(program, "instance_color");
	GLint instance_life_loc = glGetAttribLocation(program, "instance_life_data");

	// Verify all attributes exist
	if (in_position_loc < 0 || in_texcoord_loc < 0 ||
		instance_pos_scale_loc < 0 || instance_color_loc < 0)
	{
		std::cout << "Error: Missing shader attributes for instanced rendering" << std::endl;
		return;
	}
	gl_has_errors();

	// Set up vertex attributes
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
						  sizeof(TexturedVertex), (void *)0);

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE,
						  sizeof(TexturedVertex), (void *)sizeof(vec3));
	gl_has_errors();

	// Prepare instance data arrays (3 vec4s per instance: pos/scale, color, life data)
	std::vector<vec4> instance_data(particleCount * 3);

	// Fill instance data
	int idx = 0;
	for (Entity entity : registry.particles.entities)
	{
		Particle &particle = registry.particles.get(entity);

		// Get position and scale
		vec2 position = particle.Position;
		vec2 scale = vec2(10.0f * (particle.Life / particle.MaxLife));
		if (registry.motions.has(entity))
		{
			scale = registry.motions.get(entity).scale;
		}

		// Position(xy) and scale(zw)
		instance_data[idx++] = vec4(position.x, position.y, scale.x, scale.y);

		// Color with alpha
		instance_data[idx++] = particle.Color;

		// Life ratio and other parameters
		instance_data[idx++] = vec4(particle.Life / particle.MaxLife, 0.0f, 0.0f, 0.0f);
	}

	// Upload instance data
	glBindBuffer(GL_ARRAY_BUFFER, particle_instance_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, instance_data.size() * sizeof(vec4), instance_data.data());
	gl_has_errors();

	// Set up instance attributes
	// 1. Position and scale (vec4)
	glEnableVertexAttribArray(instance_pos_scale_loc);
	glVertexAttribPointer(instance_pos_scale_loc, 4, GL_FLOAT, GL_FALSE,
						  sizeof(vec4) * 3, (void *)0);
	glVertexAttribDivisor(instance_pos_scale_loc, 1); // Once per instance
	gl_has_errors();

	// 2. Color (vec4)
	glEnableVertexAttribArray(instance_color_loc);
	glVertexAttribPointer(instance_color_loc, 4, GL_FLOAT, GL_FALSE,
						  sizeof(vec4) * 3, (void *)(sizeof(vec4)));
	glVertexAttribDivisor(instance_color_loc, 1);
	gl_has_errors();

	// // 3. Life data (vec4)
	// glEnableVertexAttribArray(instance_life_loc);
	// glVertexAttribPointer(instance_life_loc, 4, GL_FLOAT, GL_FALSE,
	//                       sizeof(vec4) * 3, (void *)(sizeof(vec4) * 2));
	// glVertexAttribDivisor(instance_life_loc, 1);
	// gl_has_errors();

	if (instance_life_loc >= 0)
	{
		glEnableVertexAttribArray(instance_life_loc);
		glVertexAttribPointer(instance_life_loc, 4, GL_FLOAT, GL_FALSE,
							  sizeof(vec4) * 3, (void *)(sizeof(vec4) * 2));
		glVertexAttribDivisor(instance_life_loc, 1);
		gl_has_errors();
	}

	// Count indices
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	GLsizei num_indices = size / sizeof(uint16_t);

	// Draw all particles in one call
	glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr, particleCount);
	gl_has_errors();

	// Reset attribute divisors to avoid affecting other rendering
	glVertexAttribDivisor(instance_pos_scale_loc, 0);
	glVertexAttribDivisor(instance_color_loc, 0);
	// glVertexAttribDivisor(instance_life_loc, 0);
	gl_has_errors();

	// Disable attributes we enabled
	glDisableVertexAttribArray(instance_pos_scale_loc);
	glDisableVertexAttribArray(instance_color_loc);
	// glDisableVertexAttribArray(instance_life_loc);

	if (instance_life_loc >= 0)
	{
		glVertexAttribDivisor(instance_life_loc, 0);
		glDisableVertexAttribArray(instance_life_loc);
	}
	gl_has_errors();
}