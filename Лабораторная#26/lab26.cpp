
#include <windows.h>									
#include <gl\gl.h>										
#include <gl\glu.h>										
#include <gl\glaux.h>									
#include <stdio.h>										

HDC			hDC=NULL;									
HGLRC		hRC=NULL;									
HWND		hWnd=NULL;									
HINSTANCE	hInstance = NULL;							

bool		keys[256];									
bool		active=TRUE;								
bool		fullscreen=TRUE;							


static GLfloat LightAmb[] = {0.7f, 0.7f, 0.7f, 1.0f};	
static GLfloat LightDif[] = {1.0f, 1.0f, 1.0f, 1.0f};	
static GLfloat LightPos[] = {4.0f, 4.0f, 6.0f, 1.0f};	

GLUquadricObj	*q;										

GLfloat		xrot		=  0.0f;						
GLfloat		yrot		=  0.0f;						
GLfloat		xrotspeed	=  0.0f;						
GLfloat		yrotspeed	=  0.0f;						
GLfloat		zoom		= -7.0f;						
GLfloat		height		=  2.0f;						

GLuint		texture[3];									

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		
{
	if (height==0)										
	{
		height=1;										
	}

	glViewport(0,0,width,height);						

	glMatrixMode(GL_PROJECTION);						
	glLoadIdentity();									

	
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							
	glLoadIdentity();									
}

AUX_RGBImageRec *LoadBMP(char *Filename)				
{
	FILE *File=NULL;									

	if (!Filename)										
	{
		return NULL;									
	}

	File=fopen(Filename,"r");							

	if (File)											
	{
		fclose(File);									
		return auxDIBImageLoad(Filename);				
	}

	return NULL;										
}

int LoadGLTextures()                                    
{
    int Status=FALSE;									
    AUX_RGBImageRec *TextureImage[3];					
    memset(TextureImage,0,sizeof(void *)*3);			
    if ((TextureImage[0]=LoadBMP("Data/EnvWall.bmp")) &&
        (TextureImage[1]=LoadBMP("Data/Ball.bmp")) &&	
        (TextureImage[2]=LoadBMP("Data/EnvRoll.bmp")))	
	{   
		Status=TRUE;									
		glGenTextures(3, &texture[0]);					
		for (int loop=0; loop<3; loop++)				
		{
			glBindTexture(GL_TEXTURE_2D, texture[loop]);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[loop]->sizeX, TextureImage[loop]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[loop]->data);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		}
		for (int loop=0; loop<3; loop++)					
		{
			if (TextureImage[loop])						
			{
				if (TextureImage[loop]->data)			
				{
					free(TextureImage[loop]->data);		
				}
				free(TextureImage[loop]);				
			}
		}
	}
	return Status;										
}

int InitGL(GLvoid)										
{
	if (!LoadGLTextures())								
	{
		return FALSE;									
	}
	glShadeModel(GL_SMOOTH);							
	glClearColor(0.2f, 0.5f, 1.0f, 1.0f);				
	glClearDepth(1.0f);									
	glClearStencil(0);									
	glEnable(GL_DEPTH_TEST);							
	glDepthFunc(GL_LEQUAL);								
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	
	glEnable(GL_TEXTURE_2D);							

	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmb);			
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDif);			
	glLightfv(GL_LIGHT0, GL_POSITION, LightPos);		

	glEnable(GL_LIGHT0);								
	glEnable(GL_LIGHTING);								

	q = gluNewQuadric();								
	gluQuadricNormals(q, GL_SMOOTH);					
	gluQuadricTexture(q, GL_TRUE);						

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);	
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);	

	return TRUE;										
}

void DrawObject()										
{
	glColor3f(1.0f, 1.0f, 1.0f);						
	glBindTexture(GL_TEXTURE_2D, texture[1]);			
	gluSphere(q, 0.35f, 32, 16);						

	glBindTexture(GL_TEXTURE_2D, texture[2]);			
	glColor4f(1.0f, 1.0f, 1.0f, 0.4f);					
	glEnable(GL_BLEND);									
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);					
	glEnable(GL_TEXTURE_GEN_S);							
	glEnable(GL_TEXTURE_GEN_T);							

	gluSphere(q, 0.35f, 32, 16);						
														
	glDisable(GL_TEXTURE_GEN_S);						
	glDisable(GL_TEXTURE_GEN_T);						
	glDisable(GL_BLEND);								
}

void DrawFloor()										
{
	glBindTexture(GL_TEXTURE_2D, texture[0]);			
	glBegin(GL_QUADS);									
		glNormal3f(0.0, 1.0, 0.0);						
			glTexCoord2f(0.0f, 1.0f);					
			glVertex3f(-2.0, 0.0, 2.0);					
			
			glTexCoord2f(0.0f, 0.0f);					
			glVertex3f(-2.0, 0.0,-2.0);					
			
			glTexCoord2f(1.0f, 0.0f);					
			glVertex3f( 2.0, 0.0,-2.0);					
			
			glTexCoord2f(1.0f, 1.0f);					
			glVertex3f( 2.0, 0.0, 2.0);					
	glEnd();											
}

int DrawGLScene(GLvoid)									
{
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	
	double eqr[] = {0.0f,-1.0f, 0.0f, 0.0f};			

	glLoadIdentity();									
	glTranslatef(0.0f, -0.6f, zoom);					
	glColorMask(0,0,0,0);								
	glEnable(GL_STENCIL_TEST);							
	glStencilFunc(GL_ALWAYS, 1, 1);						
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);			
														
														
	glDisable(GL_DEPTH_TEST);							
	DrawFloor();										
														
	glEnable(GL_DEPTH_TEST);							
	glColorMask(1,1,1,1);								
	glStencilFunc(GL_EQUAL, 1, 1);						
														
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);				
	glEnable(GL_CLIP_PLANE0);							
														
	glClipPlane(GL_CLIP_PLANE0, eqr);					
	glPushMatrix();										
		glScalef(1.0f, -1.0f, 1.0f);					
		glLightfv(GL_LIGHT0, GL_POSITION, LightPos);	
		glTranslatef(0.0f, height, 0.0f);				
		glRotatef(xrot, 1.0f, 0.0f, 0.0f);				
		glRotatef(yrot, 0.0f, 1.0f, 0.0f);				
		DrawObject();									
	glPopMatrix();										
	glDisable(GL_CLIP_PLANE0);							
	glDisable(GL_STENCIL_TEST);							
	glLightfv(GL_LIGHT0, GL_POSITION, LightPos);		
	glEnable(GL_BLEND);									
	glDisable(GL_LIGHTING);								
	glColor4f(1.0f, 1.0f, 1.0f, 0.8f);					
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
	DrawFloor();										
	glEnable(GL_LIGHTING);								
	glDisable(GL_BLEND);								
	glTranslatef(0.0f, height, 0.0f);					
	glRotatef(xrot, 1.0f, 0.0f, 0.0f);					
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);					
	DrawObject();										
	xrot += xrotspeed;									
	yrot += yrotspeed;									
	glFlush();											
	return TRUE;										
}

void ProcessKeyboard()									
{
	if (keys[VK_RIGHT])		yrotspeed += 0.08f;			
	if (keys[VK_LEFT])		yrotspeed -= 0.08f;			
	if (keys[VK_DOWN])		xrotspeed += 0.08f;			
	if (keys[VK_UP])		xrotspeed -= 0.08f;			

	if (keys['A'])			zoom +=0.05f;				
	if (keys['Z'])			zoom -=0.05f;				

	if (keys[VK_PRIOR])		height +=0.03f;				
	if (keys[VK_NEXT])		height -=0.03f;				
}

GLvoid KillGLWindow(GLvoid)								
{
	if (hRC)											
	{
		if (!wglMakeCurrent(NULL,NULL))					
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										
	}

	if (hWnd && !DestroyWindow(hWnd))					
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										
	}

	if (fullscreen)										
	{
		ChangeDisplaySettings(NULL,0);					
		ShowCursor(TRUE);								
	}

	if (!UnregisterClass("OpenGL",hInstance))			
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;									
	}
}

 
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;							
	WNDCLASS	wc;										
	DWORD		dwExStyle;								
	DWORD		dwStyle;								

	fullscreen=fullscreenflag;							

	hInstance			= GetModuleHandle(NULL);		
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	
	wc.lpfnWndProc		= (WNDPROC) WndProc;			
	wc.cbClsExtra		= 0;							
	wc.cbWndExtra		= 0;							
	wc.hInstance		= hInstance;					
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);	
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);	
	wc.hbrBackground	= NULL;							
	wc.lpszMenuName		= NULL;							
	wc.lpszClassName	= "OpenGL";						

	if (!RegisterClass(&wc))							
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;									
	}
	
	if (fullscreen)										
	{
		DEVMODE dmScreenSettings;						
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);	
		dmScreenSettings.dmPelsWidth	= width;		
		dmScreenSettings.dmPelsHeight	= height;		
		dmScreenSettings.dmBitsPerPel	= bits;			
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;						
			}
			else
			{
				
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;							
			}
		}
	}

	if (fullscreen)										
	{
		dwExStyle=WS_EX_APPWINDOW;						
		dwStyle=WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;	
		ShowCursor(FALSE);								
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;	
		dwStyle=WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;	
	}

	
	if (!(hWnd=CreateWindowEx(	dwExStyle,				
								"OpenGL",				
								title,					
								dwStyle,				
								0, 0,					
								width, height,			
								NULL,					
								NULL,					
								hInstance,				
								NULL)))					
	{
		KillGLWindow();									
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;									
	}

	static	PIXELFORMATDESCRIPTOR pfd=					
	{
		sizeof(PIXELFORMATDESCRIPTOR),					
		1,												
		PFD_DRAW_TO_WINDOW |							
		PFD_SUPPORT_OPENGL |							
		PFD_DOUBLEBUFFER,								
		PFD_TYPE_RGBA,									
		bits,											
		0, 0, 0, 0, 0, 0,								
		0,												
		0,												
		0,												
		0, 0, 0, 0,										
		16,												
		1,												
		0,												
		PFD_MAIN_PLANE,									
		0,												
		0, 0, 0											
	};
	
	if (!(hDC=GetDC(hWnd)))								
	{
		KillGLWindow();									
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;									
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))		
	{
		KillGLWindow();									
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;									
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))			
	{
		KillGLWindow();									
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;									
	}

	if (!(hRC=wglCreateContext(hDC)))					
	{
		KillGLWindow();									
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;									
	}

	if(!wglMakeCurrent(hDC,hRC))						
	{
		KillGLWindow();									
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;									
	}

	ShowWindow(hWnd,SW_SHOW);							
	SetForegroundWindow(hWnd);							
	SetFocus(hWnd);										
	ReSizeGLScene(width, height);						

	if (!InitGL())										
	{
		KillGLWindow();									
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;									
	}

	return TRUE;										
}


LRESULT CALLBACK WndProc(	HWND	hWnd,				
							UINT	uMsg,				
							WPARAM	wParam,				
							LPARAM	lParam)				
{
	switch (uMsg)										
	{
		case WM_ACTIVATE:								
		{
			if (!HIWORD(wParam))						
			{
				active=TRUE;							
			}
			else										
			{
				active=FALSE;							
			}

			return 0;									
		}

		case WM_SYSCOMMAND:								
		{
			switch (wParam)								
			{
				case SC_SCREENSAVE:						
				case SC_MONITORPOWER:					
				return 0;								
			}
			break;										
		}

		case WM_CLOSE:									
		{
			PostQuitMessage(0);							
			return 0;									
		}

		case WM_KEYDOWN:								
		{
			keys[wParam] = TRUE;						
			return 0;									
		}

		case WM_KEYUP:									
		{
			keys[wParam] = FALSE;						
			return 0;									
		}

		case WM_SIZE:									
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));  
			return 0;									
		}
	}

	
	return DefWindowProc(hWnd,uMsg,wParam,lParam);		
}

int WINAPI WinMain(	HINSTANCE	hInstance,				
					HINSTANCE	hPrevInstance,			
					LPSTR		lpCmdLine,				
					int			nCmdShow)				
{
	MSG		msg;										
	BOOL	done=FALSE;									

	
	if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO)
	{
		fullscreen=FALSE;								
	}

	
	if (!CreateGLWindow("LR26", 640, 480, 32, fullscreen))
	{
		return 0;										
	}

	while(!done)										
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))		
		{
			if (msg.message==WM_QUIT)					
			{
				done=TRUE;								
			}
			else										
			{
				TranslateMessage(&msg);					
				DispatchMessage(&msg);					
			}
		}
		else											
		{
			
			if (active)									
			{
				if (keys[VK_ESCAPE])					
				{
					done=TRUE;							
				}
				else									
				{
					DrawGLScene();						
					SwapBuffers(hDC);					

					ProcessKeyboard();					
				}
			}
		}
	}

	
	KillGLWindow();										
	return (msg.wParam);								
}
