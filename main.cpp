#define GLEW_STATIC

#include <gl/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>		//Calculating view and projection matrices
#include <glm/gtc/type_ptr.hpp>				//Converting matrix object into float array for OpenGL
#include <string>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <algorithm>

#define HM_SIZE_X	40
#define HM_SIZE_Y	40
#define INDEX_SIZE	((2*HM_SIZE_X + 1)*(HM_SIZE_Y-1))
#define NUM_ATTRIB 11

std::string StringFromFile(const char* filename)
{
	std::ifstream fs(filename);
	if (!fs)
	{
		return "";
	}

	std::string s(
		std::istreambuf_iterator<char>{fs},
		std::istreambuf_iterator<char>{});

	return s;
}

int main(int argc, char *argv[]) {

	//Set up window and context
	SDL_Init(SDL_INIT_VIDEO);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	// Create the window
	SDL_Window* window = SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
	if (!window)
	{
		fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
		return 1;
	}

	// Create the OpenGL context
	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (!context)
	{
		fprintf(stderr, "SDL_GL_CreateContext: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Event windowEvent;

	//Load GLEW to enable OpenGL functionality
	glewExperimental = GL_TRUE;
	glewInit();

	// Compile vertex shader
	const char* version = "#version 150\n";
	GLint status;
	std::string vs_source = StringFromFile("scene.vert");
	const char* vs_strings[] = { version, vs_source.c_str() };
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 2, vs_strings, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		GLint logLength;
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> log(logLength + 1);
		glGetShaderInfoLog(vs, logLength, NULL, log.data());
		fprintf(stderr, "Error compiling vertex shader: %s\n", log.data());
		return 1;
	}

	// Compile fragment shader
	std::string fs_source = StringFromFile("scene.frag");
	const char* fs_strings[] = { version, fs_source.c_str() };
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 2, fs_strings, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		GLint logLength;
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> log(logLength + 1);
		glGetShaderInfoLog(fs, logLength, NULL, log.data());
		fprintf(stderr, "Error compiling fragment shader: %s\n", log.data());
		return 1;
	}

	// Link program (vertex + fragment shader)
	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glBindFragDataLocation(program, 0, "outColor");
	//Use glDrawBuffers when rendering to multiple buffers, because only the first output will be enabled by default
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status)
	{
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> log(logLength + 1);
		glGetProgramInfoLog(program, logLength, NULL, log.data());
		fprintf(stderr, "Error linking program: %s\n", log.data());
		return 1;
	}

	// Can now bind the program to the graphics pipeline, to render with it.
	glUseProgram(program);

	//Enable zBuffering
	glEnable(GL_DEPTH_TEST);

	//Create and bind Vertex Array Object (while store all links between atrtibutes and VBOs with raw vertex data)
	GLuint vao;	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);					//uiVAOHeightmap

	//Create vertex data and copy to Vertex Buffer Object
	GLuint vbo;
	glGenBuffers(1, &vbo);					
	glBindBuffer(GL_ARRAY_BUFFER, vbo);		//uiVBOHeightmapData

	//glm::vec3 vHeightmapData[HM_SIZE_X * HM_SIZE_Y];
	GLfloat vertices[HM_SIZE_X * HM_SIZE_Y * NUM_ATTRIB];

	float fHeights[HM_SIZE_X*HM_SIZE_Y] = {
		4.0f, 2.0f, 3.0f, 1.0f,
		3.0f, 5.0f, 8.0f, 2.0f,
		7.0f, 10.0f, 12.0f, 6.0f,
		4.0f, 6.0f, 8.0f, 3.0f
	};

	float fSizeX = 40.0f, fSizeZ = 40.0f;

	/*GLfloat vertices[] = {
	//X     Y      Z     R     G     B     U     V     NX    NY     NZ
	//Ar
	-1.2f, 1.2f, -0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  //ARA
	-0.4f, 1.2f, -0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,//Mid3A
	0.4f, 1.2f, -0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,//Mid1B
	1.2f, 1.2f, -0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,//ARB

	-1.2f, 0.4f, -0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,//A1A
	-0.4f, 0.4f, 0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,//A1B
	0.4f, 0.4f, -0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,//Mid1A
	1.2f, 0.4f, -0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,//Mid1B

	-1.2f, -0.4f, -0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,//Mid2A
	-0.4f, -0.4f, -0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,//Mid2B
	0.4f, -0.4f, -0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,//Mid3B
	1.2f, -0.4f, -0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,//Mid4A

	-1.2f, -1.2f, -0.6f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,//Mid4B
	-0.4f, -1.2f, -0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,//Mid5A
	0.4f, -1.2f, -0.6f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,//Mid5B
	1.2f, -1.2f, -0.6f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,//Mid6A
	};*/

	/*vHeightmapData[i] = glm::vec3(
	-fSizeX / 2 + fSizeX*x / float(HM_SIZE_X - 1),
	fHeights[i],
	-fSizeZ / 2 + fSizeZ*z / float(HM_SIZE_Y - 1)
	);*/
	for (int i = 0; i < HM_SIZE_X * HM_SIZE_Y; i++) {
		float x = float(i%HM_SIZE_X), z = float(i / HM_SIZE_X);
		int k = i * 11;
		vertices[k++] = -fSizeX / 2 + fSizeX*x / float(HM_SIZE_X - 1);		//X
		vertices[k++] = 0;//fHeights[i];										//Y
		vertices[k++] = -fSizeZ / 2 + fSizeZ*z / float(HM_SIZE_Y - 1);		//Z
		vertices[k++] = 1.0f;												//R
		vertices[k++] = 1.0f;												//G
		vertices[k++] = 0.0f;												//B
		vertices[k++] = 0.0f;												//U
		vertices[k++] = 0.0f;												//V
		vertices[k++] = 0.0f;												//NX
		vertices[k++] = 0.0f;												//NY
		vertices[k++] = 0.0f;												//NZ
	}
	//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*HM_SIZE_X*HM_SIZE_Y, vHeightmapData, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*HM_SIZE_X*HM_SIZE_Y*NUM_ATTRIB, vertices, GL_STATIC_DRAW);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint uiVBOIndices;
	glGenBuffers(1, &uiVBOIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiVBOIndices);

	int iIndices[INDEX_SIZE];

	int count = 0;
	for (int j = 0; j < HM_SIZE_Y - 1; j++) {
		for (int i = 0; i < 2*HM_SIZE_X; i+=2) {
			iIndices[count++] = (j*HM_SIZE_X + i / 2);
			iIndices[count++] = ((j+1)*HM_SIZE_X + i/2);
		}
		iIndices[count++] = -1;
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iIndices), iIndices, GL_STATIC_DRAW);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(-1);

	/*int iIndices[] = {
	0, 4, 1, 5, 2, 6, 3, 7, -1,
	4, 8, 5, 9, 6, 10, 7, 11, -1,
	8, 12, 9, 13, 10, 14, 11, 15
	};*/

	// -- Setup drawing attributes --
	//Input, # values (# components of vec), type of component, bool for -1 -> 1 normalized,
	//stride (bytes between each position attribute), offset (how many bytes from start the attribute occurs)
	//Will also store the VBO bound to GL_ARRAY_BUFFER - can use a different VBO for each attribute

	
	//Position attribute
	GLint posAttrib = glGetAttribLocation(program, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(posAttrib);

	//Color attribute
	GLint colAttrib = glGetAttribLocation(program, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	//Texture coordinate attribute
	GLint texAttrib = glGetAttribLocation(program, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

	//Normal attribute
	GLint normAttrib = glGetAttribLocation(program, "normal");
	glEnableVertexAttribArray(normAttrib);
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void *)(8 * sizeof(GLfloat)));


	//Texture
	/*
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	int width, height;
	unsigned char* image = SOIL_load_image("img.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	//Texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// Texture X
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	// Texture Y
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);		// Scaled down
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);		// Scale up
	*/
	
	//Model matrix
	GLint uniModel = glGetUniformLocation(program, "model");

	//View matrix
	glm::mat4 view = glm::lookAt(
		glm::vec3(2.5f, 2.5f, 80.0f),			//Position of camera
		glm::vec3(0.0f, 0.0f, 0.0f),			//Point centered on screen
		glm::vec3(0.0f, 0.0f, 1.0f)				//Up axis (x,y is the ground)
		);
	GLint uniView = glGetUniformLocation(program, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	//Projection matrix: vertical FOV, aspect ratio, near plane, far plane (for clipping)
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 100.0f);
	GLint uniProj = glGetUniformLocation(program, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	//Uniform to decrease brightness of reflection
	GLint uniColor = glGetUniformLocation(program, "overrideColor");

	//Main event loop
	float direction;
	float xMovement = 0.0f;
	float yMovement = 0.8f;		//Start at appropriate zoom
	float zMovement = 0.0f;
	
	while (true) {

		
		//Movement events
		if (SDL_PollEvent(&windowEvent)) {
			if (windowEvent.type == SDL_QUIT) break;

			//Keyboard presses
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_LEFT) {
				xMovement = fmodf(xMovement - 0.1f, -2.0f);
			}
			if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_RIGHT){
				xMovement = fmodf(xMovement + 0.1f, 2.0f);
			}

			//Mouse - trolley
			if (windowEvent.type == SDL_MOUSEMOTION && windowEvent.button.button == SDL_BUTTON(SDL_BUTTON_MIDDLE)){
				direction = windowEvent.motion.yrel;
				if (direction > 0) {
					yMovement = std::max(yMovement + 0.05f, 0.0f);
				}
				else if (direction < 0) {
					yMovement = std::max(yMovement - 0.05f, 0.0f);
				}
			}

			//Mouse - rotation
			if (windowEvent.type == SDL_MOUSEMOTION && windowEvent.button.button == SDL_BUTTON(SDL_BUTTON_LEFT)){
				direction = windowEvent.motion.xrel;
				if (direction > 0) {
					xMovement = fmodf(xMovement + 0.01f, 2.0f);
				}
				else if (direction < 0){
					xMovement = fmodf(xMovement - 0.01f, -2.0f);
				}
			}

			//Mouse - viewing angle
			if (windowEvent.type == SDL_MOUSEMOTION && windowEvent.button.button == SDL_BUTTON(SDL_BUTTON_RIGHT)){
				direction = windowEvent.motion.yrel;
				if (direction > 0) {
					zMovement = fmodf(zMovement - 0.01f, -2.0f);
				}
				else if (direction < 0) {
					zMovement = fmodf(zMovement + 0.01f, 2.0f);
				}
			}
		}

		//Clear screen to black
		glClearColor(0.1f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Rotation Transformation
		glm::mat4 model;
		model = glm::rotate(model, xMovement * glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, zMovement * glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

		//Draw
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_TRIANGLE_STRIP, HM_SIZE_X*(HM_SIZE_Y - 1) * 2 + (HM_SIZE_Y - 2), GL_UNSIGNED_INT, 0);

		/*
		//Draw regular cube
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Drawing mirror (w/ stencil buffer)
		glEnable(GL_STENCIL_TEST);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glDepthMask(GL_FALSE);
		glDrawArrays(GL_TRIANGLES, 36, 6);


		//Draw inverted cube
		glStencilFunc(GL_EQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDepthMask(GL_TRUE);

		model = glm::scale(
			glm::translate(model, glm::vec3(0, 0, -1)),
			glm::vec3(1, 1, -1)
			);
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3f(uniColor, 0.1f, 0.1f, 0.1f);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glUniform3f(uniColor, 1.0f, 1.0f, 1.0f);

		glDisable(GL_STENCIL_TEST);

		//Floor grid
		glDepthMask(GL_FALSE);
		glDrawArrays(GL_LINES, 42, 20);
		glDepthMask(GL_TRUE);
		*/

		//Swap buffers
		SDL_GL_SwapWindow(window);
	}
	

	SDL_Delay(100);

	//Unbind from graphics pipeline
	//glDeleteTextures(1, &tex);
	glDeleteProgram(program);
	glDeleteShader(fs);
	glDeleteShader(vs);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &uiVBOIndices);
	glDeleteVertexArrays(1, &vao);

	SDL_GL_DeleteContext(context);
	SDL_Quit();
	return 0;
}