#include <windows.h>											
#include <gl\gl.h>												
#include <gl\glu.h>												
#include <gl\glaux.h>											
#include "NeHeGL.h"												
#include <math.h>												


#include "arb_multisample.h"
bool domulti = true;
bool doangle = true;


#pragma comment( lib, "opengl32.lib" )							
#pragma comment( lib, "glu32.lib" )								
#pragma comment( lib, "glaux.lib" )								

#ifndef CDS_FULLSCREEN											
#define CDS_FULLSCREEN 4										
#endif															

GL_Window*	g_window;
Keys*		g_keys;

float angle= 0.0f;

BOOL Initialize (GL_Window* window, Keys* keys)					
{
	g_window	= window;
	g_keys		= keys;

	
	angle		= 0.0f;											

	glViewport(0 , 0,window->init.width ,window->init.height);	
	glMatrixMode(GL_PROJECTION);								
	glLoadIdentity();											
	gluPerspective(50, (float)window->init.width/(float)window->init.height, 5,  2000); 
	glMatrixMode(GL_MODELVIEW);									
	glLoadIdentity();											

	glEnable(GL_DEPTH_TEST);									

	glShadeModel(GL_SMOOTH);									

	glClearColor(0.0f, 0.0f, 0.0f, 0.5);						

	return TRUE;												
}

void Deinitialize (void)										
{
}

void Update (DWORD milliseconds)								
{
	
	if(g_keys->keyDown [VK_SPACE] == TRUE)
		domulti=!domulti;

	if(g_keys->keyDown ['M'] == TRUE)
		doangle=!doangle;
	

	if (g_keys->keyDown [VK_ESCAPE] == TRUE)					
	{
		TerminateApplication (g_window);						
	}

	if (g_keys->keyDown [VK_F1] == TRUE)						
	{
		ToggleFullscreen (g_window);							
	}
}

void Draw (void)												
{
	
	if(domulti)
		glEnable(GL_MULTISAMPLE_ARB);							
	

	glClearColor(0.0f, 0.0f, 0.0f, 0.5);						
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
	glLoadIdentity();											

	for(float i=-10;i<10;i++)
		for(float j=-10;j<10;j++)
		{
			glPushMatrix();
			glTranslatef(i*2.0f,j*2.0f,-5.0f);
			glRotatef(angle,0.f,0.f,1.f);
				glBegin(GL_QUADS);
				glColor3f(1.0f,0.0f,0.0f);glVertex3f(i,j,0.0f);
				glColor3f(0.0f,1.0f,0.0f);glVertex3f(i + 2.0f,j,0.0f);
				glColor3f(0.0f,0.0f,1.0f);glVertex3f(i + 2.0f,j + 2.0f,0.0f);
				glColor3f(1.0f,1.0f,1.0f);glVertex3f(i,j + 2.0f,0.0f);
				glEnd();
			glPopMatrix();
		}

	if(doangle)
		angle+=0.05f;

	glFlush ();													

	
	if(domulti)
		glDisable(GL_MULTISAMPLE_ARB);
	
}
