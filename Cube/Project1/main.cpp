#pragma comment(lib,"glew-2.1.0\\lib\\Release\\Win32\\glew32.lib")
#pragma comment(lib,"glfw-3.2.1\\lib-vc2015\\glfw3.lib")
#pragma comment(lib,"opengl32.lib")

#include "glew-2.1.0\include\GL\glew.h"
#include "glfw-3.2.1/include/GLFW/glfw3.h"
#include <iostream>
#include <cstdlib>

//screen resolution
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

//draw and key press functions
void keyCallback( GLFWwindow *window, int key, int scancode, int action, int mods );
void DrawCube( GLfloat centerPosX, GLfloat centerPosY, GLfloat centerPosZ, GLfloat edgeLength );

//rotation for x and y
GLfloat rotationX = 0.0f;
GLfloat rotationY = 0.0f;

int main( void )
{
    GLFWwindow *window;
    
	// init glfw library
	if (!glfwInit())
	{
		return -1;
	}
    
    // screen init
    window = glfwCreateWindow( SCREEN_WIDTH, SCREEN_HEIGHT, "The Cube", NULL, NULL );
    
    glfwSetKeyCallback( window, keyCallback );
    glfwSetInputMode( window, GLFW_STICKY_KEYS, 1 );
    
    
    int screenWidth, screenHeight;
    glfwGetFramebufferSize( window, &screenWidth, &screenHeight );

    if ( !window )
    {
        glfwTerminate( );
        return -1;
    }
   
    glfwMakeContextCurrent( window );
    
    glViewport( 0.0f, 0.0f, screenWidth, screenHeight ); //sets where gl draws.
    glMatrixMode( GL_PROJECTION );  //sets view of camera on cube
    glOrtho( 0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, 0, 800 );
    glMatrixMode( GL_MODELVIEW ); // set metrix default
    glLoadIdentity( );
    GLfloat halfScreenWidth = SCREEN_WIDTH / 2;
    GLfloat halfScreenHeight = SCREEN_HEIGHT / 2;
    
    // running loop until program is closed
    while ( !glfwWindowShouldClose( window ) )
    {
        glClear( GL_COLOR_BUFFER_BIT );
        
      //rendering
        
        glPushMatrix( );
        glTranslatef( halfScreenWidth, halfScreenHeight, -500 );
        glRotatef( rotationX, 1, 0, 0 );
        glRotatef( rotationY, 0, 1, 0 );
        glTranslatef( -halfScreenWidth, -halfScreenHeight, 500 );
        
        DrawCube( halfScreenWidth, halfScreenHeight, -500, 250 );
		//DrawCube(halfScreenWidth, halfScreenHeight, -500, 50); //this is if you want to add a samll cube in the middle of the cube. (must set draw to line and not to fill. option is at the bottom.)
        
        glPopMatrix();
        glfwSwapBuffers( window );
        glfwPollEvents( );
    }
        
    glfwTerminate( );
    
    return 0;
}

//adds controls to rotate the cube.
void keyCallback( GLFWwindow *window, int key, int scancode, int action, int mods )
{
	//added to key presses to rotate the cube.
    const GLfloat rotationSpeed = 15;

    if ( action == GLFW_PRESS || action == GLFW_REPEAT )
    {
        switch ( key )
        {
			//all key presses add or subtract 15 rotation speed to move the cube.
            case GLFW_KEY_UP:
                rotationX -= rotationSpeed;
                break;
            case GLFW_KEY_DOWN:
                rotationX += rotationSpeed;
                break;
            case GLFW_KEY_RIGHT:
                rotationY += rotationSpeed;
                break;
            case GLFW_KEY_LEFT:
                rotationY -= rotationSpeed;
                break;
        } 
    }
}

void DrawCube( GLfloat centerPosX, GLfloat centerPosY, GLfloat centerPosZ, GLfloat edgeLength )
{
    GLfloat halfLength = edgeLength * 0.5f;
    
    GLfloat vertices[] =
    {
        // front face  
        centerPosX - halfLength, centerPosY + halfLength, centerPosZ + halfLength, // top left edge 
        centerPosX + halfLength, centerPosY + halfLength, centerPosZ + halfLength, // top right edge
        centerPosX + halfLength, centerPosY - halfLength, centerPosZ + halfLength, // bottom right edge
        centerPosX - halfLength, centerPosY - halfLength, centerPosZ + halfLength, // bottom left edge (this goes for each face same order.)
        
	   // back face
	   centerPosX - halfLength, centerPosY + halfLength, centerPosZ - halfLength,
	   centerPosX + halfLength, centerPosY + halfLength, centerPosZ - halfLength,
	   centerPosX + halfLength, centerPosY - halfLength, centerPosZ - halfLength,
	   centerPosX - halfLength, centerPosY - halfLength, centerPosZ - halfLength,

	   // right face
	  centerPosX + halfLength, centerPosY + halfLength, centerPosZ + halfLength,
	  centerPosX + halfLength, centerPosY + halfLength, centerPosZ - halfLength,
	  centerPosX + halfLength, centerPosY - halfLength, centerPosZ - halfLength,
	  centerPosX + halfLength, centerPosY - halfLength, centerPosZ + halfLength,

		// left face
	   centerPosX - halfLength, centerPosY + halfLength, centerPosZ + halfLength,
	   centerPosX - halfLength, centerPosY + halfLength, centerPosZ - halfLength,
	   centerPosX - halfLength, centerPosY - halfLength, centerPosZ - halfLength,
	   centerPosX - halfLength, centerPosY - halfLength, centerPosZ + halfLength,

	   // top face
	  centerPosX - halfLength, centerPosY + halfLength, centerPosZ + halfLength,
	  centerPosX - halfLength, centerPosY + halfLength, centerPosZ - halfLength,
	  centerPosX + halfLength, centerPosY + halfLength, centerPosZ - halfLength,
	  centerPosX + halfLength, centerPosY + halfLength, centerPosZ + halfLength,

	   // bottom face
	   centerPosX - halfLength, centerPosY - halfLength, centerPosZ + halfLength,
	   centerPosX - halfLength, centerPosY - halfLength, centerPosZ - halfLength,
	   centerPosX + halfLength, centerPosY - halfLength, centerPosZ - halfLength,
	   centerPosX + halfLength, centerPosY - halfLength, centerPosZ + halfLength
    };
    
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); //shows a wire frame
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // fills cube
    glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays( GL_QUADS, 0, 24 );
    glDisableClientState( GL_VERTEX_ARRAY );
}