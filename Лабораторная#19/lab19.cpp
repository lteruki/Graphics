
#include <windows.h>				
#include <stdio.h>					
#include <gl\gl.h>					
#include <gl\glu.h>					
#include <gl\glaux.h>				

#define	MAX_PARTICLES	1000		

HDC			hDC=NULL;				
HGLRC		hRC=NULL;				
HWND		hWnd=NULL;				
HINSTANCE	hInstance;				

bool	keys[256];					
bool	active=TRUE;				
bool	fullscreen=TRUE;			
bool	rainbow=true;				
bool	sp;							
bool	rp;							

float	slowdown=2.0f;				
float	xspeed;						
float	yspeed;						
float	zoom=-40.0f;				

GLuint	loop;						
GLuint	col;						
GLuint	delay;						
GLuint	texture[1];					

typedef struct						
{
	bool	active;					
	float	life;					
	float	fade;					
	float	r;						
	float	g;						
	float	b;						
	float	x;						
	float	y;						
	float	z;						
	float	xi;						
	float	yi;						
	float	zi;						
	float	xg;						
	float	yg;						
	float	zg;						
}
particles;							

particles particle[MAX_PARTICLES];	

static GLfloat colors[12][3]=		
{
	{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
	{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
	{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
};

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	

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
        AUX_RGBImageRec *TextureImage[1];				
        memset(TextureImage,0,sizeof(void *)*1);		

        if (TextureImage[0]=LoadBMP("Data/Particle.bmp"))	
        {
			Status=TRUE;								
			glGenTextures(1, &texture[0]);				

			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
        }

        if (TextureImage[0])							
		{
			if (TextureImage[0]->data)					
			{
				free(TextureImage[0]->data);			
			}
			free(TextureImage[0]);						
		}
        return Status;									
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		
{
	if (height==0)										
	{
		height=1;										
	}

	glViewport(0,0,width,height);						

	glMatrixMode(GL_PROJECTION);						
	glLoadIdentity();									

	
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,200.0f);

	glMatrixMode(GL_MODELVIEW);							
	glLoadIdentity();									
}

int InitGL(GLvoid)										
{
	if (!LoadGLTextures())								
	{
		return FALSE;									
	}

	glShadeModel(GL_SMOOTH);							
	glClearColor(0.0f,0.0f,0.0f,0.0f);					
	glClearDepth(1.0f);									
	glDisable(GL_DEPTH_TEST);							
	glEnable(GL_BLEND);									
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);					
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);	
	glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);				
	glEnable(GL_TEXTURE_2D);							
	glBindTexture(GL_TEXTURE_2D,texture[0]);			

	for (loop=0;loop<MAX_PARTICLES;loop++)				
	{
		particle[loop].active=true;								
		particle[loop].life=1.0f;								
		particle[loop].fade=float(rand()%100)/1000.0f+0.003f;	
		particle[loop].r=colors[loop*(12/MAX_PARTICLES)][0];	
		particle[loop].g=colors[loop*(12/MAX_PARTICLES)][1];	
		particle[loop].b=colors[loop*(12/MAX_PARTICLES)][2];	
		particle[loop].xi=float((rand()%50)-26.0f)*10.0f;		
		particle[loop].yi=float((rand()%50)-25.0f)*10.0f;		
		particle[loop].zi=float((rand()%50)-25.0f)*10.0f;		
		particle[loop].xg=0.0f;									
		particle[loop].yg=-0.8f;								
		particle[loop].zg=0.0f;									
	}

	return TRUE;										
}

int DrawGLScene(GLvoid)										
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
	glLoadIdentity();										

	for (loop=0;loop<MAX_PARTICLES;loop++)					
	{
		if (particle[loop].active)							
		{
			float x=particle[loop].x;						
			float y=particle[loop].y;						
			float z=particle[loop].z+zoom;					

			
			glColor4f(particle[loop].r,particle[loop].g,particle[loop].b,particle[loop].life);

			glBegin(GL_TRIANGLE_STRIP);						
			    glTexCoord2d(1,1); glVertex3f(x+0.5f,y+0.5f,z); 
				glTexCoord2d(0,1); glVertex3f(x-0.5f,y+0.5f,z); 
				glTexCoord2d(1,0); glVertex3f(x+0.5f,y-0.5f,z); 
				glTexCoord2d(0,0); glVertex3f(x-0.5f,y-0.5f,z); 
			glEnd();										

			particle[loop].x+=particle[loop].xi/(slowdown*1000);
			particle[loop].y+=particle[loop].yi/(slowdown*1000);
			particle[loop].z+=particle[loop].zi/(slowdown*1000);

			particle[loop].xi+=particle[loop].xg;			
			particle[loop].yi+=particle[loop].yg;			
			particle[loop].zi+=particle[loop].zg;			
			particle[loop].life-=particle[loop].fade;		

			if (particle[loop].life<0.0f)					
			{
				particle[loop].life=1.0f;					
				particle[loop].fade=float(rand()%100)/1000.0f+0.003f;	
				particle[loop].x=0.0f;						
				particle[loop].y=0.0f;						
				particle[loop].z=0.0f;						
				particle[loop].xi=xspeed+float((rand()%60)-32.0f);	
				particle[loop].yi=yspeed+float((rand()%60)-30.0f);	
				particle[loop].zi=float((rand()%60)-30.0f);	
				particle[loop].r=colors[col][0];			
				particle[loop].g=colors[col][1];			
				particle[loop].b=colors[col][2];			
			}

			
			if (keys[VK_NUMPAD8] && (particle[loop].yg<1.5f)) particle[loop].yg+=0.01f;

			
			if (keys[VK_NUMPAD2] && (particle[loop].yg>-1.5f)) particle[loop].yg-=0.01f;

			
			if (keys[VK_NUMPAD6] && (particle[loop].xg<1.5f)) particle[loop].xg+=0.01f;

			
			if (keys[VK_NUMPAD4] && (particle[loop].xg>-1.5f)) particle[loop].xg-=0.01f;

			if (keys[VK_TAB])										
			{
				particle[loop].x=0.0f;								
				particle[loop].y=0.0f;								
				particle[loop].z=0.0f;								
				particle[loop].xi=float((rand()%50)-26.0f)*10.0f;	
				particle[loop].yi=float((rand()%50)-25.0f)*10.0f;	
				particle[loop].zi=float((rand()%50)-25.0f)*10.0f;	
			}
		}
    }
	return TRUE;											
}

GLvoid KillGLWindow(GLvoid)								
{
	if (fullscreen)										
	{
		ChangeDisplaySettings(NULL,0);					
		ShowCursor(TRUE);								
	}

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
	RECT		WindowRect;				
	WindowRect.left=(long)0;			
	WindowRect.right=(long)width;		
	WindowRect.top=(long)0;				
	WindowRect.bottom=(long)height;		

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
		dwStyle=WS_POPUP;										
		ShowCursor(FALSE);										
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			
		dwStyle=WS_OVERLAPPEDWINDOW;							
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		

	
	if (!(hWnd=CreateWindowEx(	dwExStyle,							
								"OpenGL",							
								title,								
								dwStyle |							
								WS_CLIPSIBLINGS |					
								WS_CLIPCHILDREN,					
								0, 0,								
								WindowRect.right-WindowRect.left,	
								WindowRect.bottom-WindowRect.top,	
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
		0,											
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

	
	if (!CreateGLWindow("LR19",640,480,16,fullscreen))
	{
		return 0;									
	}

	if (fullscreen)									
	{
		slowdown=1.0f;								
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
			
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	
			{
				done=TRUE;							
			}
			else									
			{
				SwapBuffers(hDC);					

				if (keys[VK_ADD] && (slowdown>1.0f)) slowdown-=0.01f;		
				if (keys[VK_SUBTRACT] && (slowdown<4.0f)) slowdown+=0.01f;	

				if (keys[VK_PRIOR])	zoom+=0.1f;		
				if (keys[VK_NEXT])	zoom-=0.1f;		

				if (keys[VK_RETURN] && !rp)			
				{
					rp=true;						
					rainbow=!rainbow;				
				}
				if (!keys[VK_RETURN]) rp=false;		
				
				if ((keys[' '] && !sp) || (rainbow && (delay>25)))	
				{
					if (keys[' '])	rainbow=false;	
					sp=true;						
					delay=0;						
					col++;							
					if (col>11)	col=0;				
				}
				if (!keys[' '])	sp=false;			

				
				if (keys[VK_UP] && (yspeed<200)) yspeed+=1.0f;

				
				if (keys[VK_DOWN] && (yspeed>-200)) yspeed-=1.0f;

				
				if (keys[VK_RIGHT] && (xspeed<200)) xspeed+=1.0f;

				
				if (keys[VK_LEFT] && (xspeed>-200)) xspeed-=1.0f;

				delay++;							

				if (keys[VK_F1])						
				{
					keys[VK_F1]=FALSE;					
					KillGLWindow();						
					fullscreen=!fullscreen;				
					
					if (!CreateGLWindow("NeHe's Particle Tutorial",640,480,16,fullscreen))
					{
						return 0;						
					}
				}
			}
		}
	}

	
	KillGLWindow();									
	return (msg.wParam);							
}
