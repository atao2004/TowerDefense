// stdlib
#include <iostream>
#include <sstream>
#include <array>
#include <fstream>

// internal
#include "../ext/stb_image/stb_image.h"
#include "render_system.hpp"
#include "tinyECS/registry.hpp"

#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_VERTEX_LAYOUT
#define NK_IMPLEMENTATION
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "../ext/nuklear.h"
#include "../ext/nuklear_glfw_gl3.h"


// #define NK_INCLUDE_FIXED_TYPES
// #include <nuklear.h>
// #include <nuklear_glfw_gl3.h>

// Render initialization
bool RenderSystem::init(GLFWwindow *window_arg)
{
	// struct nk_font_atlas atlas;  
	this->window = window_arg;

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // vsync

	// Load OpenGL function pointers
	const int is_fine = gl3w_init();
	assert(is_fine == 0);

	// Create a frame buffer
	frame_buffer = 0;
	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// For some high DPI displays (ex. Retina Display on Macbooks)
	// https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
	int frame_buffer_width_px, frame_buffer_height_px;
	glfwGetFramebufferSize(window, &frame_buffer_width_px, &frame_buffer_height_px); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	if (frame_buffer_width_px != WINDOW_WIDTH_PX)
	{
		printf("WARNING: retina display! https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value\n");
		printf("glfwGetFramebufferSize = %d,%d\n", frame_buffer_width_px, frame_buffer_height_px);
		printf("requested window width,height = %d,%d\n", WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX);
	}

	// Hint: Ask your TA for how to setup pretty OpenGL error callbacks.
	// This can not be done in mac os, so do not enable
	// it unless you are on Linux or Windows. You will need to change the window creation
	// code to use OpenGL 4.3 (not suported on mac) and add additional .h and .cpp
	// glDebugMessageCallback((GLDEBUGPROC)errorCallback, nullptr);

	// We are not really using VAO's but without at least one bound we will crash in
	// some systems.
	//GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	gl_has_errors();

	initScreenTexture();
	initializeGlTextures();
	initializeGlEffects();
	initializeGlGeometryBuffers();

	struct nk_glfw glfw;
	ctx = nk_glfw3_init(&glfw, const_cast<GLFWwindow *>(window), NK_GLFW3_INSTALL_CALLBACKS);
	// Load default font
	struct nk_font_atlas* atlas;
	nk_glfw3_font_stash_begin(&glfw,&atlas);
	nk_font* font = nk_font_atlas_add_default(atlas, 13.0f, NULL);
	nk_glfw3_font_stash_end(&glfw);

	// Assign the font to the context's style
	ctx->style.font = (const struct nk_user_font*)font;

	std::string font_filename = PROJECT_SOURCE_DIR + std::string("data/fonts/Kenney_Mini_Square.ttf");
	unsigned int font_default_size = 100;
	std::cout << "Loading font from: " << font_filename << std::endl;
	std::ifstream fontFile(font_filename);
	if (!fontFile.good())
	{
		std::cerr << "Font file not found!" << std::endl;
	}
	fontInit(font_filename, font_default_size);

	return true;
}

// void RenderSystem::initializeGlTextures()
// {
//     glGenTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());

//     for(uint i = 0; i < texture_paths.size(); i++)
//     {
// 		const std::string& path = texture_paths[i];
// 		ivec2& dimensions = texture_dimensions[i];

// 		stbi_uc* data;
// 		data = stbi_load(path.c_str(), &dimensions.x, &dimensions.y, NULL, 4);

// 		if (data == NULL)
// 		{
// 			const std::string message = "Could not load the file " + path + ".";
// 			fprintf(stderr, "%s", message.c_str());
// 			assert(false);
// 		}
// 		glBindTexture(GL_TEXTURE_2D, texture_gl_handles[i]);
// 		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimensions.x, dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 		gl_has_errors();
// 		stbi_image_free(data);
//     }
// 	gl_has_errors();
// }

void RenderSystem::initializeGlTextures()
{
	glGenTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());

	for (uint i = 0; i < texture_paths.size(); i++)
	{
		const std::string &path = texture_paths[i];
		ivec2 &dimensions = texture_dimensions[i];

		// Check if this is a skeleton texture that needs scaling
		bool is_skeleton_texture = false;
		if (i >= (uint)TEXTURE_ASSET_ID::SKELETON_IDLE1 &&
			i <= (uint)TEXTURE_ASSET_ID::SKELETON_IDLE6)
		{
			is_skeleton_texture = true;
		}

		stbi_uc *data;
		data = stbi_load(path.c_str(), &dimensions.x, &dimensions.y, NULL, 4);

		if (data == NULL)
		{
			const std::string message = "Could not load the file " + path + ".";
			fprintf(stderr, "%s", message.c_str());
			assert(false);
		}

		// If this is a skeleton texture, scale it up 5x
		if (is_skeleton_texture)
		{
			// Calculate new dimensions
			int original_width = dimensions.x;
			int original_height = dimensions.y;
			int new_width = original_width * 5;
			int new_height = original_height * 5;

			// Allocate memory for scaled texture
			stbi_uc *scaled_data = new stbi_uc[new_width * new_height * 4];

			// Simple nearest-neighbor scaling
			for (int y = 0; y < new_height; y++)
			{
				for (int x = 0; x < new_width; x++)
				{
					int src_x = x / 5;
					int src_y = y / 5;

					// Copy pixel data (RGBA = 4 channels)
					for (int c = 0; c < 4; c++)
					{
						scaled_data[(y * new_width + x) * 4 + c] =
							data[(src_y * original_width + src_x) * 4 + c];
					}
				}
			}

			// Upload scaled texture to GPU
			glBindTexture(GL_TEXTURE_2D, texture_gl_handles[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, new_width, new_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled_data);

			// Update dimensions
			dimensions.x = new_width;
			dimensions.y = new_height;

			// Free memory
			delete[] scaled_data;
			stbi_image_free(data);
		}
		else
		{
			// Regular texture processing
			glBindTexture(GL_TEXTURE_2D, texture_gl_handles[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimensions.x, dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}

		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl_has_errors();
	}
	gl_has_errors();
}

void RenderSystem::initializeGlEffects()
{
	for (uint i = 0; i < effect_paths.size(); i++)
	{
		const std::string vertex_shader_name = effect_paths[i] + ".vs.glsl";
		const std::string fragment_shader_name = effect_paths[i] + ".fs.glsl";

		bool is_valid = loadEffectFromFile(vertex_shader_name, fragment_shader_name, effects[i]);
		assert(is_valid && (GLuint)effects[i] != 0);
	}
}

// One could merge the following two functions as a template function...
template <class T>
void RenderSystem::bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices)
{
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(uint)gid]);
	glBufferData(GL_ARRAY_BUFFER,
				 sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	gl_has_errors();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(uint)gid]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
	gl_has_errors();
}

void RenderSystem::initializeGlMeshes()
{
	for (uint i = 0; i < mesh_paths.size(); i++)
	{
		// Initialize meshes
		GEOMETRY_BUFFER_ID geom_index = mesh_paths[i].first;
		std::string name = mesh_paths[i].second;
		Mesh::loadFromOBJFile(name,
							  meshes[(int)geom_index].vertices,
							  meshes[(int)geom_index].vertex_indices,
							  meshes[(int)geom_index].original_size);

		bindVBOandIBO(geom_index,
					  meshes[(int)geom_index].vertices,
					  meshes[(int)geom_index].vertex_indices);
	}
}

void RenderSystem::initializeGlGeometryBuffers()
{
	// Vertex Buffer creation.
	glGenBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
	// Index Buffer creation.
	glGenBuffers((GLsizei)index_buffers.size(), index_buffers.data());

	// Index and Vertex buffer data initialization.
	initializeGlMeshes();

	//////////////////////////
	// Initialize sprite
	// The position corresponds to the center of the texture.
	std::vector<TexturedVertex> textured_vertices(4);
	textured_vertices[0].position = {-1.f / 2, +1.f / 2, 0.f};
	textured_vertices[1].position = {+1.f / 2, +1.f / 2, 0.f};
	textured_vertices[2].position = {+1.f / 2, -1.f / 2, 0.f};
	textured_vertices[3].position = {-1.f / 2, -1.f / 2, 0.f};
	textured_vertices[0].texcoord = {0.f, 1.f};
	textured_vertices[1].texcoord = {1.f, 1.f};
	textured_vertices[2].texcoord = {1.f, 0.f};
	textured_vertices[3].texcoord = {0.f, 0.f};

	// Counterclockwise as it's the default OpenGL front winding direction.
	const std::vector<uint16_t> textured_indices = {0, 3, 1, 1, 3, 2};
	bindVBOandIBO(GEOMETRY_BUFFER_ID::SPRITE, textured_vertices, textured_indices);

	/* LEGACY - not used, but code below still relies on it...*/
	////////////////////////
	// Initialize egg
	std::vector<ColoredVertex> egg_vertices;
	std::vector<uint16_t> egg_indices;
	constexpr float z = -0.1f;
	constexpr int NUM_TRIANGLES = 62;

	for (int i = 0; i < NUM_TRIANGLES; i++)
	{
		const float t = float(i) * M_PI * 2.f / float(NUM_TRIANGLES - 1);
		egg_vertices.push_back({});
		egg_vertices.back().position = {0.5 * cos(t), 0.5 * sin(t), z};
		egg_vertices.back().color = {0.8, 0.8, 0.8};
	}
	egg_vertices.push_back({});
	egg_vertices.back().position = {0, 0, 0};
	egg_vertices.back().color = {1, 1, 1};
	for (int i = 0; i < NUM_TRIANGLES; i++)
	{
		egg_indices.push_back((uint16_t)i);
		egg_indices.push_back((uint16_t)((i + 1) % NUM_TRIANGLES));
		egg_indices.push_back((uint16_t)NUM_TRIANGLES);
	}
	int geom_index = (int)GEOMETRY_BUFFER_ID::EGG;
	meshes[geom_index].vertices = egg_vertices;
	meshes[geom_index].vertex_indices = egg_indices;
	bindVBOandIBO(GEOMETRY_BUFFER_ID::EGG, meshes[geom_index].vertices, meshes[geom_index].vertex_indices);

	//////////////////////////////////
	// Initialize debug line
	std::vector<ColoredVertex> line_vertices;
	std::vector<uint16_t> line_indices;

	constexpr float depth = 0.5f;
	// constexpr vec3 red = { 0.8, 0.1, 0.1 };
	constexpr vec3 red = {1.0, 1.0, 1.0};

	// Corner points
	line_vertices = {
		{{-0.5, -0.5, depth}, red},
		{{-0.5, 0.5, depth}, red},
		{{0.5, 0.5, depth}, red},
		{{0.5, -0.5, depth}, red},
	};

	// Two triangles
	line_indices = {0, 1, 3, 1, 2, 3};

	geom_index = (int)GEOMETRY_BUFFER_ID::DEBUG_LINE;
	meshes[geom_index].vertices = line_vertices;
	meshes[geom_index].vertex_indices = line_indices;
	bindVBOandIBO(GEOMETRY_BUFFER_ID::DEBUG_LINE, line_vertices, line_indices);

	///////////////////////////////////////////////////////
	// Initialize screen triangle (yes, triangle, not quad; its more efficient).
	std::vector<vec3> screen_vertices(3);
	screen_vertices[0] = {-1, -6, 0.f};
	screen_vertices[1] = {6, -1, 0.f};
	screen_vertices[2] = {-1, 6, 0.f};

	// Counterclockwise as it's the default opengl front winding direction.
	const std::vector<uint16_t> screen_indices = {0, 1, 2};
	bindVBOandIBO(GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE, screen_vertices, screen_indices);
}

RenderSystem::~RenderSystem()
{
	// Don't need to free gl resources since they last for as long as the program,
	// but it's polite to clean after yourself.
	glDeleteBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
	glDeleteBuffers((GLsizei)index_buffers.size(), index_buffers.data());
	glDeleteTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());
	glDeleteTextures(1, &off_screen_render_buffer_color);
	glDeleteRenderbuffers(1, &off_screen_render_buffer_depth);
	gl_has_errors();

	for (uint i = 0; i < effect_count; i++)
	{
		glDeleteProgram(effects[i]);
	}
	// delete allocated resources
	glDeleteFramebuffers(1, &frame_buffer);
	gl_has_errors();

	// remove all entities created by the render system
	while (registry.renderRequests.entities.size() > 0)
		registry.remove_all_components_of(registry.renderRequests.entities.back());
}

// Initialize the screen texture from a standard sprite
bool RenderSystem::initScreenTexture()
{
	// create a single entry
	registry.screenStates.emplace(screen_state_entity);

	int framebuffer_width, framebuffer_height;
	glfwGetFramebufferSize(const_cast<GLFWwindow *>(window), &framebuffer_width, &framebuffer_height); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	glGenTextures(1, &off_screen_render_buffer_color);
	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl_has_errors();

	glGenRenderbuffers(1, &off_screen_render_buffer_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, off_screen_render_buffer_depth);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, off_screen_render_buffer_color, 0);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebuffer_width, framebuffer_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, off_screen_render_buffer_depth);
	gl_has_errors();

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	return true;
}

bool gl_compile_shader(GLuint shader)
{
	glCompileShader(shader);
	gl_has_errors();
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint log_len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
		std::vector<char> log(log_len);
		glGetShaderInfoLog(shader, log_len, &log_len, log.data());
		glDeleteShader(shader);

		gl_has_errors();

		fprintf(stderr, "GLSL: %s", log.data());
		return false;
	}

	return true;
}

bool loadEffectFromFile(
	const std::string &vs_path, const std::string &fs_path, GLuint &out_program)
{
	// Opening files
	std::ifstream vs_is(vs_path);
	std::ifstream fs_is(fs_path);
	if (!vs_is.good() || !fs_is.good())
	{
		fprintf(stderr, "Failed to load shader files %s, %s", vs_path.c_str(), fs_path.c_str());
		assert(false);
		return false;
	}

	// Reading sources
	std::stringstream vs_ss, fs_ss;
	vs_ss << vs_is.rdbuf();
	fs_ss << fs_is.rdbuf();
	std::string vs_str = vs_ss.str();
	std::string fs_str = fs_ss.str();
	const char *vs_src = vs_str.c_str();
	const char *fs_src = fs_str.c_str();
	GLsizei vs_len = (GLsizei)vs_str.size();
	GLsizei fs_len = (GLsizei)fs_str.size();

	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs_src, &vs_len);
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs_src, &fs_len);
	gl_has_errors();

	// Compiling
	if (!gl_compile_shader(vertex))
	{
		fprintf(stderr, "Vertex compilation failed");
		assert(false);
		return false;
	}
	if (!gl_compile_shader(fragment))
	{
		fprintf(stderr, "Fragment compilation failed");
		assert(false);
		return false;
	}

	// Linking
	out_program = glCreateProgram();
	glAttachShader(out_program, vertex);
	glAttachShader(out_program, fragment);
	glLinkProgram(out_program);
	gl_has_errors();

	{
		GLint is_linked = GL_FALSE;
		glGetProgramiv(out_program, GL_LINK_STATUS, &is_linked);
		if (is_linked == GL_FALSE)
		{
			GLint log_len;
			glGetProgramiv(out_program, GL_INFO_LOG_LENGTH, &log_len);
			std::vector<char> log(log_len);
			glGetProgramInfoLog(out_program, log_len, &log_len, log.data());
			gl_has_errors();

			fprintf(stderr, "Link error: %s", log.data());
			assert(false);
			return false;
		}
	}

	// No need to carry this around. Keeping these objects is only useful if we recycle
	// the same shaders over and over, which we don't, so no need and this is simpler.
	glDetachShader(out_program, vertex);
	glDetachShader(out_program, fragment);
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	gl_has_errors();

	return true;
}
