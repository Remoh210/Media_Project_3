///*
//==================================================================================================================================================================
//|Copyright 2018 Oscar Lara	- scarlara@hotmail.com																												   |
//|																																								   |
//|See individual libraries separate legal notices																												   |
//|																																								   |
//|Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"),				   |
//|to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,				   |
//|and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :					   |
//|																																								   |
//|The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.									   |
//|																																								   |
//|THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,			   |
//|FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,	   |
//|WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.		   |
//==================================================================================================================================================================
//*/
//


#include <stdio.h>
#include <assert.h>
#include <vector>

#include <glad/glad.h>
#include <GLFW\glfw3.h>

#include <fmod.hpp>
#include <fmod_errors.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
//#pragma comment(lib, "freetype235.lib")

#include "assets/src/utils/gl_math.h"
#include "assets/src/utils/camera.h"
#include "assets/src/utils/cobject.h"


#define NUM_OF_SOUNDS 4


//DEFINE NUMBER OF TAGS AND STREAM BUFFER SIZE
#define NUMBER_OF_TAGS 4
//buffer size 64 * 1024
#define STREAM_BUFFER_SIZE 65536


//Define your radio stations
const char *_url_japanese_pop = "http://sky1.torontocast.com:9029";
const char *_url_japanese_60s = "http://sky1.torontocast.com:9033";
const char *_url_jrock = "http://sky1.torontocast.com:9088";

//create openstate and tags globals
FMOD_OPENSTATE _openstate = FMOD_OPENSTATE_READY;
int _tag_index = 0;
char _tag_string[NUMBER_OF_TAGS][128] = { 0 };



//Globals
unsigned int _vertex_buffer, _vertex_shader, _fragment_shader, _program;
unsigned int _view_matrix_location;
unsigned int _projection_matrix_location;
unsigned int _model_matrix_location;
unsigned int _color_location;
unsigned int cur = 0;
int _element_buffer_size;
char* _vertex_shader_text;
char* _fragment_shader_text;

Camera _camera;
double _elapsed_seconds = 0.0;
bool _camera_moved = false;
std::vector<CObject> _cobjects;


FT_Library _ft;
FT_Face _face;
unsigned int _text_program;
unsigned int _uniform_tex;
unsigned int _attribute_coord;
unsigned int _dp_vbo;


struct point {
	GLfloat x;
	GLfloat y;
	GLfloat s;
	GLfloat t;
};

//Create sound settings variables.
unsigned int position = 0;
unsigned int percentage = 0;
bool is_playing = false;
bool is_paused = false;
bool is_starving = false;
bool cur_paused = false;
const char* current_state = "Stopped";
FMOD_TAG tag;

//=============================================================

//GLFW
int _window_width = 640;
int _window_height = 480;
GLFWwindow *_main_window = NULL;

//FMOD Globals
FMOD_RESULT _result = FMOD_OK;
FMOD::System *_system = NULL;
FMOD::Sound *_sound[NUM_OF_SOUNDS];
FMOD::Channel *_channel[NUM_OF_SOUNDS];




//Functions
bool init_gl();
bool init_fmod();
bool shutdown_fmod();
bool init_text();
void render_text(const char *text, float x, float y, float sx, float sy);
bool init_others();

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		_channel[cur]->getPaused(&cur_paused);
		if (cur_paused == false)
		{
			_result = _channel[cur]->setPaused(true);
			assert(!_result);
			cur_paused = true;
		}
		else if (cur_paused == true)
		{
			_result = _channel[cur]->setPaused(false);
			assert(!_result);
			cur_paused = false;
		}

	}


	
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		if (cur != 0) {
			_result = _channel[cur]->setPaused(true);
			assert(!_result);
		}

		cur = 0;
		_channel[cur]->getPaused(&cur_paused);

		if (cur_paused == true) {
			_channel[cur]->setPaused(false);

			assert(!_result);
		}
		printf("Staion (J-Pop 60s) is playing..\n");
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		if (cur != 1) {
			_result = _channel[cur]->setPaused(true);
			assert(!_result);
		}
		cur = 1;
		_channel[cur]->getPaused(&cur_paused);
		if (cur_paused == true) {
			_channel[cur]->setPaused(false);
			assert(!_result);
		}
		printf("Staion 2 (J-Pop) is playing..\n");
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		if (cur != 2) {
			_result = _channel[cur]->setPaused(true);
			assert(!_result);
		}

		cur = 2;
		_channel[cur]->getPaused(&cur_paused);

		if (cur_paused == true) {
			_channel[cur]->setPaused(false);

			assert(!_result);
		}
		printf("Staion 3 (J-Rock) is playing..\n");
	}
}

int main() {



	fprintf(stdout, "Init opengl ...\n");
	assert(init_gl());
	fprintf(stdout, "Init fmod...\n");
	assert(init_fmod());
	fprintf(stdout, "Init text...\n");
	assert(init_text());
	fprintf(stdout, "Init others...\n");
	assert(init_others());


	//=======================================================================================
	//FMOD

	//TODO4: CREATE SOUNDS.
	_result = _system->createSound(_url_japanese_60s, FMOD_CREATESTREAM | FMOD_NONBLOCKING, 0, &_sound[0]);
	assert(!_result);
	_result = _system->createSound(_url_japanese_pop, FMOD_CREATESTREAM | FMOD_NONBLOCKING, 0, &_sound[1]);
	assert(!_result);
	_result = _system->createSound(_url_jrock, FMOD_CREATESTREAM | FMOD_NONBLOCKING, 0, &_sound[2]);
	assert(!_result);

	//_system->playSound(_sound[cur], 0, true, &_channel[cur])




	//=======================================================================================
	fprintf(stdout, "Ready ...!\n");

	std::vector<CObject>::iterator itObjects;

	float sx = 2.0f / _window_width;
	float sy = 2.0f / _window_height;
	GLfloat yoffset = 30.0f;
	GLfloat xoffset = 8 * sx;

	char text_buffer[512];
	float offset_step = 40.0f;

	while (!glfwWindowShouldClose(_main_window)) {
		yoffset = 30.0f;
		static double previous_seconds = glfwGetTime();
		double current_seconds = glfwGetTime();
		_elapsed_seconds = current_seconds - previous_seconds;
		previous_seconds = current_seconds;

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwGetFramebufferSize(_main_window, &_window_width, &_window_height);
		glViewport(0, 0, _window_width, _window_height);

		render_text("Text to speach", -1 + xoffset, 1 - yoffset * sy, sx * 0.5, sy * 0.5);


		//TODO6: 
		//Get stream sound state
		_result = _sound[cur]->getOpenState(&_openstate, &percentage, &is_starving, 0);
		assert(!_result);

		if (_channel[cur]) {
			//TODO7:
			//Retrieve descriptive tag stored by the sound
			while (_sound[cur]->getTag(0, -1, &tag) == FMOD_OK)
			{
				if (tag.datatype == FMOD_TAGDATATYPE_STRING) {
					sprintf(_tag_string[_tag_index], "%s = %s (%d bytes)", tag.name, (char *)tag.data, tag.datalen);
					_tag_index = (_tag_index + 1) % NUMBER_OF_TAGS;
				}
				else
				{
					//when a song changes, the sample rate migth also change
					float frequency = *((float *)tag.data);
					_result = _channel[cur]->setFrequency(frequency);
					assert(!_result);
				}

			}

			//TODO8: Get channel settings
			_result = _channel[cur]->getPaused(&is_paused);
			assert(!_result);
			_result = _channel[cur]->isPlaying(&is_playing);
			assert(!_result);
			_result = _channel[cur]->getPosition(&position, FMOD_TIMEUNIT_MS);
			assert(!_result);
			_result = _channel[cur]->setMute(is_starving);
			assert(!_result);

		}
		else
		{
			//TODO9: Play sound
			//this might fail if the stream is not ready, so do not check for errors.
			_system->playSound(_sound[cur], 0, false, &_channel[cur]);
		}



		//TODO10: 
		//Check stream/sound settings
		if (_openstate == FMOD_OPENSTATE_CONNECTING) {
			current_state = "Connecting...";
		}
		else if (_openstate == FMOD_OPENSTATE_BUFFERING) {
			current_state = "Buffering...";
		}
		else if (is_paused) {
			current_state = "Paused...";
		}
		else if (is_playing) {
			current_state = "Playing...";
		}


		//TODO11: PRINT SOME TEXT
		render_text("Media Fundamentals... streaming from the internet...", -1 + xoffset, 1 - yoffset * sy, sx * 0.5, sy * 0.5);
		yoffset += offset_step;
		sprintf(text_buffer, "time: %02d:%02d:%02d", position / 1000 / 60, position / 100 % 60, position / 10 % 100);
		render_text(text_buffer, -1 + xoffset, 1 - yoffset * sy, sx * 0.5, sy * 0.5);
		yoffset += offset_step;
		sprintf(text_buffer, "Current state: %s %s", current_state, is_starving ? " (STARVING)" : "");
		render_text(text_buffer, -1 + xoffset, 1 - yoffset * sy, sx * 0.5, sy * 0.5);
		yoffset += offset_step;
		sprintf(text_buffer, "Buffer percentage: %d", percentage);
		render_text(text_buffer, -1 + xoffset, 1 - yoffset * sy, sx * 0.5, sy * 0.5);
		yoffset += offset_step;
		//Print tags
		for (int i = _tag_index; i < (_tag_index + NUMBER_OF_TAGS); i++)
		{
			sprintf(text_buffer, "%s", _tag_string[i%NUMBER_OF_TAGS]);
			render_text(text_buffer, -1 + xoffset, 1 - yoffset * sy, sx * 0.5, sy * 0.5);
			yoffset += offset_step;
		}

		glfwSwapBuffers(_main_window);
		glfwPollEvents();

		_result = _system->update();
		assert(!_result);
	}

	glfwDestroyWindow(_main_window);
	glfwTerminate();

	assert(shutdown_fmod());

	return 0;
}


bool init_others() {

	//=======================================================================================
	//Shaders
	int params = -1;


	//Read shaders from files
	FILE *f = fopen("assets/src/shaders/simple_light.vert", "rb");
	fseek(f, 0, SEEK_END);
	long filesize = ftell(f);
	fseek(f, 0, SEEK_SET);
	char* simple_light_vert = (char*)malloc(filesize + 1);
	fread(simple_light_vert, filesize, 1, f);
	fclose(f);
	simple_light_vert[filesize] = 0;
	//--
	f = fopen("assets/src/shaders/simple_light.frag", "rb");
	fseek(f, 0, SEEK_END);
	filesize = ftell(f);
	fseek(f, 0, SEEK_SET);
	char* simple_light_frag = (char*)malloc(filesize + 1);
	fread(simple_light_frag, filesize, 1, f);
	fclose(f);
	simple_light_frag[filesize] = 0;
	//--
	fprintf(stdout, "Creating, compiling vertex shader ...\n");
	_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(_vertex_shader, 1, &simple_light_vert, NULL);
	glCompileShader(_vertex_shader);
	glGetShaderiv(_vertex_shader, GL_COMPILE_STATUS, &params);
	if (!params) {
		fprintf(stderr, "Unable to compile vertex shader");
		return 1;
	}
	fprintf(stdout, "Creating, compiling fragment shader ...\n");
	_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(_fragment_shader, 1, &simple_light_frag, NULL);
	glCompileShader(_fragment_shader);
	glGetShaderiv(_fragment_shader, GL_COMPILE_STATUS, &params);
	if (!params) {
		fprintf(stderr, "Unable to compile fragment shader");
		return 1;
	}

	_program = glCreateProgram();
	glAttachShader(_program, _vertex_shader);
	glAttachShader(_program, _fragment_shader);
	glLinkProgram(_program);

	glGetProgramiv(_program, GL_LINK_STATUS, &params);
	if (!params) {
		fprintf(stderr, "Unable to link shader program");
		return 1;
	}
	//--	
	f = fopen("assets/src/shaders/simple_text.vert", "rb");
	fseek(f, 0, SEEK_END);
	filesize = ftell(f);
	fseek(f, 0, SEEK_SET);
	char* simple_text_vert = (char*)malloc(filesize + 1);
	fread(simple_text_vert, filesize, 1, f);
	fclose(f);
	simple_text_vert[filesize] = 0;
	//--
	f = fopen("assets/src/shaders/simple_text.frag", "rb");
	fseek(f, 0, SEEK_END);
	filesize = ftell(f);
	fseek(f, 0, SEEK_SET);
	char* simple_text_frag = (char*)malloc(filesize + 1);
	fread(simple_text_frag, filesize, 1, f);
	fclose(f);
	simple_text_frag[filesize] = 0;

	//text vertex shader
	unsigned int simple_text_vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(simple_text_vs, 1, &simple_text_vert, NULL);
	glCompileShader(simple_text_vs);
	glGetShaderiv(simple_text_vs, GL_COMPILE_STATUS, &params);
	if (!params) {
		fprintf(stderr, "Unable to compile simple text vertex shader");
		return 1;
	}
	unsigned int simple_text_fg = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(simple_text_fg, 1, &simple_text_frag, NULL);
	glCompileShader(simple_text_fg);
	glGetShaderiv(simple_text_fg, GL_COMPILE_STATUS, &params);
	if (!params) {
		fprintf(stderr, "Unable to compile simple text fragment shader");
		return 1;
	}

	_text_program = glCreateProgram();
	glAttachShader(_text_program, simple_text_vs);
	glAttachShader(_text_program, simple_text_fg);
	glLinkProgram(_text_program);

	glGetProgramiv(_text_program, GL_LINK_STATUS, &params);
	if (!params) {
		fprintf(stderr, "Unable to link simple text shader program");
		return 1;
	}

	_uniform_tex = glGetUniformLocation(_text_program, "tex");
	_attribute_coord = glGetAttribLocation(_text_program, "coord");

	glGenBuffers(1, &_dp_vbo);



	//=======================================================================================
	float points[] = {
		// front
		-1.0, -1.0,  1.0,
		1.0, -1.0,  1.0,
		1.0,  1.0,  1.0,
		-1.0,  1.0,  1.0,
		// back
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0,  1.0, -1.0,
		-1.0,  1.0, -1.0,
	};
	GLushort cube_elements[] = {
		// front
		0, 1, 2,
		2, 3, 0,
		// right
		1, 5, 6,
		6, 2, 1,
		// back
		7, 6, 5,
		5, 4, 7,
		// left
		4, 0, 3,
		3, 7, 4,
		// bottom
		4, 5, 1,
		1, 0, 4,
		// top
		3, 2, 6,
		6, 7, 3,
	};

	unsigned int points_vbo;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	unsigned int elememnts_buffer;
	glGenBuffers(1, &elememnts_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elememnts_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &_element_buffer_size);


	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elememnts_buffer);
	glEnableVertexAttribArray(0);



	_view_matrix_location = glGetUniformLocation(_program, "view");
	_projection_matrix_location = glGetUniformLocation(_program, "proj");
	_model_matrix_location = glGetUniformLocation(_program, "model");
	_color_location = glGetUniformLocation(_program, "vertex_colour");

	glUseProgram(_program);
	glUniformMatrix4fv(_view_matrix_location, 1, false, _camera.get_view_matrix().m);
	glUniformMatrix4fv(_projection_matrix_location, 1, false, _camera.get_projection_matrix().m);
	glUniformMatrix4fv(_model_matrix_location, 1, false, mat4().get_identity().m);

	return true;
}

bool init_gl() {

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		fprintf(stderr, "Unable to init glfw\n");
		return false;
	}


	_main_window = glfwCreateWindow(1024, 768, "Media Fundamentals... stream from the internet", NULL, NULL);

	if (!_main_window)
	{
		glfwTerminate();
		fprintf(stderr, "Unable to create main window glfw\n");
		return false;
	}
	glfwSetKeyCallback(_main_window, key_callback);
	glfwMakeContextCurrent(_main_window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "Unable to init glad\n");
		return false;
	}
	glfwSwapInterval(1);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_TRUE);


	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

	return true;
}

bool init_fmod() {

	//Create the main system object
	_result = FMOD::System_Create(&_system);
	//TODO: CHECK FOR FMOD ERRORS, IMPLEMENT YOUR OWN FUNCTION
	assert(!_result);
	//Initializes the system object, and the msound device. This has to be called at the start of the user's program
	_result = _system->init(512, FMOD_INIT_NORMAL, NULL);
	assert(!_result);


	return true;
}

bool init_text() {

	if (FT_Init_FreeType(&_ft)) {
		fprintf(stderr, "Unable to init text...\n");
		return false;
	}
	if (FT_New_Face(_ft, "assets/fonts/arial.ttf", 0, &_face)) {
		fprintf(stderr, "Unable to init text...\n");
		return false;
	}

	//Font size
	FT_Set_Pixel_Sizes(_face, 0, 36);

	if (FT_Load_Char(_face, 'X', FT_LOAD_RENDER))
	{
		fprintf(stderr, "unable to load character\n");
		return false;
	}

	return true;
}

void render_text(const char *text, float x, float y, float sx, float sy) {

	glUseProgram(_text_program);

	const char *p;
	FT_GlyphSlot g = _face->glyph;

	GLuint tex;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(_uniform_tex, 0);

	/* We require 1 byte alignment when uploading texture data */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* Clamping to edges is important to prevent artifacts when scaling */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Linear filtering usually looks best for text */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Set up the VBO for our vertex data */
	glEnableVertexAttribArray(_attribute_coord);
	glBindBuffer(GL_ARRAY_BUFFER, _dp_vbo);
	glVertexAttribPointer(_attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);


	/* Loop through all characters */
	for (p = text; *p; p++) {
		/* Try to load and render the character */
		if (FT_Load_Char(_face, *p, FT_LOAD_RENDER))
			continue;


		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			g->bitmap.width,
			g->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			g->bitmap.buffer
		);

		/* Calculate the vertex and texture coordinates */
		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;

		point box[4] = {
			{ x2, -y2, 0, 0 },
		{ x2 + w, -y2, 1, 0 },
		{ x2, -y2 - h, 0, 1 },
		{ x2 + w, -y2 - h, 1, 1 },
		};


		/* Draw the character on the screen */
		glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Advance the cursor to the start of the next character */
		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;
	}
	glDisableVertexAttribArray(_attribute_coord);
	glDeleteTextures(1, &tex);
	glEnableVertexAttribArray(0);

}

bool shutdown_fmod() {



	if (_channel[0])
	{
		_result = _channel[0]->stop();
		assert(!_result);
	}
	//TODO12: Wait for sound to finish opening before trying to release it

	do
	{
		_result = _system->update();
		assert(!_result);

		_result = _sound[0]->getOpenState(&_openstate, 0, 0, 0);
		assert(!_result);

		Sleep(50);

	} while (_openstate != FMOD_OPENSTATE_READY);


	for (unsigned int i = 0; i < NUM_OF_SOUNDS; i++)
	{
		if (_sound[i]) {
			_result = _sound[i]->release();
			assert(!_result);
		}
	}


	if (_system) {
		_result = _system->close();
		assert(!_result);
		_result = _system->release();
		assert(!_result);
	}

	return true;
}