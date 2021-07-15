#include <glad\glad.h>
#include <GLFW\glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <iostream>
#include <array>
#include <thread>
#include <chrono>

#include "settings.h"
#include "models_description.h"
#include "shader.h"
#include "camera.h"

void framebuffer_size_callback( GLFWwindow* window, int width, int height );
void scroll_callback( GLFWwindow* window, double xoffset, double yoffset );
void mouse_callback( GLFWwindow* window, double xpos, double ypos );

void initializeWindow();
void initializeTextures( std::vector<unsigned int>& textures );

void processInput() noexcept;
void setUpWindow() noexcept;
void bindTextures( std::vector<unsigned int>& textures ) noexcept;

GLFWwindow* G_pWindow = nullptr;
std::unique_ptr<Camera> G_pCamera = nullptr;

int main()
{
	std::unique_ptr<Shader> defaultShader;
	std::vector<unsigned int> textures;

	G_pCamera.reset( new Camera() );

	try {
		initializeWindow();
		defaultShader.reset( new Shader( SHADER_VERTEX_FILEPATH, SHADER_FRAGMENT_FILEPATH ) );
		initializeTextures( textures );
	}
	catch ( std::exception& e ) {
		std::cerr << e.what() << std::endl;
		std::cerr << "Class: " << typeid( e ).name() << std::endl;
		glfwTerminate();
		return -1;
	}

	setUpWindow();
	glEnable( GL_DEPTH_TEST );

	unsigned int VBO;
	unsigned int VAO;
	glGenVertexArrays( 1, &VAO );
	glBindVertexArray( VAO );
	//set vertexes and copy to VRAM
	glGenBuffers( 1, &VBO );
	glBindBuffer( GL_ARRAY_BUFFER, VBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices.data(), GL_STATIC_DRAW );
	//set vertex atributes pointers
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void* )0 );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void* )( 3 * sizeof( float ) ) );
	glEnableVertexAttribArray( 1 );

	bindTextures( textures );
	defaultShader->use();
	glUniform1i( glGetUniformLocation( defaultShader->getID(), "texture1" ), 0 );
	glUniform1i( glGetUniformLocation( defaultShader->getID(), "texture2" ), 1 );

	int transformMatLocation = glGetUniformLocation( defaultShader->getID(), "transform" );

	//main loop
	while ( !glfwWindowShouldClose( G_pWindow ) ) {
		double frameStart = glfwGetTime();

		//input
		processInput();

		//rendering commands
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glBindVertexArray( VAO );
		for ( auto& eachPossition : positions ) {
			glm::mat4 trans = glm::mat4( 1.0f );
			glm::mat4 model = glm::translate( glm::mat4( 1.f ), eachPossition );
			model = glm::rotate( model,
								 glm::length( eachPossition ) * ( float )glfwGetTime() * glm::radians( 50.f ),
								 glm::vec3( 0.5f, 1.f, 0.f ) );
			glm::mat4 view = G_pCamera->getViewMatrix();

			glm::mat4 projection = glm::perspective( G_pCamera->getFov(), 800.f / 600.f, 0.1f, 100.f );
			trans = projection * view * model;

			glUniformMatrix4fv( transformMatLocation, 1, GL_FALSE, glm::value_ptr( trans ) );
			glDrawArrays( GL_TRIANGLES, 0, vertices.size() );
		}

		//check and call events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers( G_pWindow );

		double deltaTime = glfwGetTime() - frameStart;
		double updateTimeSec = 1. / ( float )FPS;

		if ( deltaTime < updateTimeSec )
			std::this_thread::sleep_for( std::chrono::milliseconds( ( ( int )( ( updateTimeSec - deltaTime ) * 1000. ) ) ) );
	}

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback( GLFWwindow* window, int width, int height )
{
	glViewport( 0, 0, width, height );
}

void scroll_callback( GLFWwindow* window, double xoffset, double yoffset )
{
	G_pCamera->ProcessMouseScroll( yoffset );
}
void mouse_callback( GLFWwindow* window, double xpos, double ypos )
{
	G_pCamera->ProcessMouseMovement( xpos, ypos );
}

void processInput() noexcept
{
	if ( glfwGetKey( G_pWindow, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
		glfwSetWindowShouldClose( G_pWindow, true );
	if ( glfwGetKey( G_pWindow, GLFW_KEY_W ) == GLFW_PRESS )
		G_pCamera->ProcessKeyboard( Camera::Camera_Movement::FORWARD, FPS );
	if ( glfwGetKey( G_pWindow, GLFW_KEY_S ) == GLFW_PRESS )
		G_pCamera->ProcessKeyboard( Camera::Camera_Movement::BACKWARD, FPS );
	if ( glfwGetKey( G_pWindow, GLFW_KEY_A ) == GLFW_PRESS )
		G_pCamera->ProcessKeyboard( Camera::Camera_Movement::LEFT, FPS );
	if ( glfwGetKey( G_pWindow, GLFW_KEY_D ) == GLFW_PRESS )
		G_pCamera->ProcessKeyboard( Camera::Camera_Movement::RIGHT, FPS );
}

void initializeWindow()
{
	//initialize and set GLFW	
	glfwInit();
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	//create window
	G_pWindow = glfwCreateWindow( 800, 600, "OpenGL TESTS", NULL, NULL );
	if ( !G_pWindow )
		throw ( std::runtime_error( std::string( "INIT::GLFW::Falied to create GLFW window!" ) ) );
	glfwMakeContextCurrent( G_pWindow );

	//initialize GLAD
	if ( !gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress ) )
		throw ( std::runtime_error( std::string( "INIT::GLAD::Failed to initialize GLAD!" ) ) );

}

void setUpWindow() noexcept
{
	glfwSetFramebufferSizeCallback( G_pWindow, framebuffer_size_callback );
	glClearColor( 0.5f, 0.5f, 0.5f, 1.f );

	glfwSetInputMode( G_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
	glfwSetCursorPosCallback( G_pWindow, mouse_callback );
	glfwSetScrollCallback( G_pWindow, scroll_callback );
}

void processEmptyLoadedTextureData( unsigned char* data )
{
	data = new unsigned char[3];
	std::fill_n( data, 3, 255 );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
	delete[] data;
	data = nullptr;
}

void processCorrectlyLoadedTexureData( const std::string& loadedFileName,
									   int& width, int& height, unsigned char* data )
{
	if ( loadedFileName == TEXTURES_FILENAMES[0] )
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
	else if ( loadedFileName == TEXTURES_FILENAMES[1] )
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
	else
		throw( std::runtime_error( "INIT::TEXTURE::Failes to recognize how to load texture into openGL!" ) );
}

void initializeTextures( std::vector<unsigned int>& textures )
{
	const unsigned short textures_count = TEXTURES_FILENAMES.size();

	textures.clear();
	textures.resize( textures_count );
	glGenTextures( textures_count, textures.data() );

	int width, height, nrChannels;
	unsigned char* data;
	stbi_set_flip_vertically_on_load( true );
	for ( size_t i = 0; i < textures_count; i++ ) {
		glBindTexture( GL_TEXTURE_2D, textures[i] );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		data = nullptr;
		try {
			data = stbi_load( TEXTURES_FILENAMES[i].c_str(),
							  &width, &height, &nrChannels, 0 );
			if ( !data )
				throw ( TEXTURES_FILENAMES[i] );
			processCorrectlyLoadedTexureData( TEXTURES_FILENAMES[i], width, height, data );
			glGenerateMipmap( GL_TEXTURE_2D );
			stbi_image_free( data );
		}
		catch ( const std::string& fileName ) {
			std::cerr << "INIT::TEXTURE::Undable to load texture from file: " << fileName << std::endl;
			processEmptyLoadedTextureData( data );
		}
	}
}

void bindTextures( std::vector<unsigned int>& textures ) noexcept
{
	for ( size_t i = 0; i < textures.size(); i++ ) {
		glActiveTexture( GL_TEXTURE0 + i );
		glBindTexture( GL_TEXTURE_2D, textures[i] );
	}
}







