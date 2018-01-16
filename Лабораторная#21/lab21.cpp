
#include <windows.h>										
#include <stdio.h>											
#include <stdarg.h>											
#include <gl\gl.h>											
#include <gl\glu.h>											
#include <gl\glaux.h>										

HDC			hDC=NULL;										
HGLRC		hRC=NULL;										
HWND		hWnd=NULL;										
HINSTANCE	hInstance;										

bool	keys[256];											
bool	vline[11][10];										
bool	hline[10][11];										
bool	ap;													
bool	filled;												
bool	gameover;											
bool	anti=TRUE;											
bool	active=TRUE;										
bool	fullscreen=TRUE;									

int		loop1;												
int		loop2;												
int		delay;												
int		adjust=3;											
int		lives=5;											
int		level=1;											
int		level2=level;										
int		stage=1;											

struct	object												
{
	int		fx, fy;											
	int		x, y;											
	float	spin;											
};

struct	object player;										
struct	object enemy[9];									
struct	object hourglass;									

struct			 											
{
  __int64       frequency;									
  float         resolution;									
  unsigned long mm_timer_start;								
  unsigned long mm_timer_elapsed;							
  bool			performance_timer;							
  __int64       performance_timer_start;					
  __int64       performance_timer_elapsed;					
} timer;													

int		steps[6]={ 1, 2, 4, 5, 10, 20 };					

GLuint	texture[2];											
GLuint	base;												

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);		

void TimerInit(void)										
{
	memset(&timer, 0, sizeof(timer));						

	
	
	if (!QueryPerformanceFrequency((LARGE_INTEGER *) &timer.frequency))
	{
		
		timer.performance_timer	= FALSE;					
		timer.mm_timer_start	= timeGetTime();			
		timer.resolution		= 1.0f/1000.0f;				
		timer.frequency			= 1000;						
		timer.mm_timer_elapsed	= timer.mm_timer_start;		
	}
	else
	{
		
		
		QueryPerformanceCounter((LARGE_INTEGER *) &timer.performance_timer_start);
		timer.performance_timer			= TRUE;				
		
		timer.resolution				= (float) (((double)1.0f)/((double)timer.frequency));
		
		timer.performance_timer_elapsed	= timer.performance_timer_start;
	}
}

float TimerGetTime()										
{
	__int64 time;											

	if (timer.performance_timer)							
	{
		QueryPerformanceCounter((LARGE_INTEGER *) &time);	
		
		return ( (float) ( time - timer.performance_timer_start) * timer.resolution)*1000.0f;
	}
	else
	{
		
		return( (float) ( timeGetTime() - timer.mm_timer_start) * timer.resolution)*1000.0f;
	}
}

void ResetObjects(void)										
{
	player.x=0;												
	player.y=0;												
	player.fx=0;											
	player.fy=0;											

	for (loop1=0; loop1<(stage*level); loop1++)				
	{
		enemy[loop1].x=5+rand()%6;							
		enemy[loop1].y=rand()%11;							
		enemy[loop1].fx=enemy[loop1].x*60;					
		enemy[loop1].fy=enemy[loop1].y*40;					
	}
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
        AUX_RGBImageRec *TextureImage[2];					
        memset(TextureImage,0,sizeof(void *)*2);			

        if ((TextureImage[0]=LoadBMP("Data/Font.bmp")) &&	
			(TextureImage[1]=LoadBMP("Data/Image.bmp")))	
        {
			Status=TRUE;									

			glGenTextures(2, &texture[0]);					

			for (loop1=0; loop1<2; loop1++)					
			{
				glBindTexture(GL_TEXTURE_2D, texture[loop1]);
				glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[loop1]->sizeX, TextureImage[loop1]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[loop1]->data);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			}

			for (loop1=0; loop1<2; loop1++)					
			{
				if (TextureImage[loop1])					
				{
					if (TextureImage[loop1]->data)			
					{
						free(TextureImage[loop1]->data);	
					}
					free(TextureImage[loop1]);				
				}
			}
		}
	return Status;											
}

GLvoid BuildFont(GLvoid)									
{
	base=glGenLists(256);									
	glBindTexture(GL_TEXTURE_2D, texture[0]);				
	for (loop1=0; loop1<256; loop1++)						
	{
		float cx=float(loop1%16)/16.0f;						
		float cy=float(loop1/16)/16.0f;						

		glNewList(base+loop1,GL_COMPILE);					
			glBegin(GL_QUADS);								
				glTexCoord2f(cx,1.0f-cy-0.0625f);			
				glVertex2d(0,16);							
				glTexCoord2f(cx+0.0625f,1.0f-cy-0.0625f);	
				glVertex2i(16,16);							
				glTexCoord2f(cx+0.0625f,1.0f-cy);			
				glVertex2i(16,0);							
				glTexCoord2f(cx,1.0f-cy);					
				glVertex2i(0,0);							
			glEnd();										
			glTranslated(15,0,0);							
		glEndList();										
	}														
}

GLvoid KillFont(GLvoid)										
{
	glDeleteLists(base,256);								
}

GLvoid glPrint(GLint x, GLint y, int set, const char *fmt, ...)	
{
	char		text[256];									
	va_list		ap;											

	if (fmt == NULL)										
		return;												

	va_start(ap, fmt);										
	    vsprintf(text, fmt, ap);							
	va_end(ap);												

	if (set>1)												
	{
		set=1;												
	}
	glEnable(GL_TEXTURE_2D);								
	glLoadIdentity();										
	glTranslated(x,y,0);									
	glListBase(base-32+(128*set));							

	if (set==0)												
	{
		glScalef(1.5f,2.0f,1.0f);							
	}

	glCallLists(strlen(text),GL_UNSIGNED_BYTE, text);		
	glDisable(GL_TEXTURE_2D);								
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

	glOrtho(0.0f,width,height,0.0f,-1.0f,1.0f);				

	glMatrixMode(GL_MODELVIEW);								
	glLoadIdentity();										
}

int InitGL(GLvoid)											
{
	if (!LoadGLTextures())									
	{
		return FALSE;										
	}

	BuildFont();											

	glShadeModel(GL_SMOOTH);								
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);					
	glClearDepth(1.0f);										
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);					
	glEnable(GL_BLEND);										
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		
	return TRUE;											
}

int DrawGLScene(GLvoid)										
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
	glBindTexture(GL_TEXTURE_2D, texture[0]);				
	glColor3f(1.0f,0.5f,1.0f);								
	glPrint(207,24,0,"GRID CRAZY");							
	glColor3f(1.0f,1.0f,0.0f);								
	glPrint(20,20,1,"Level:%2i",level2);					
	glPrint(20,40,1,"Stage:%2i",stage);						

	if (gameover)											
	{
		glColor3ub(rand()%255,rand()%255,rand()%255);		
		glPrint(472,20,1,"GAME OVER");						
		glPrint(456,40,1,"PRESS SPACE");					
	}

	for (loop1=0; loop1<lives-1; loop1++)					
	{
		glLoadIdentity();									
		glTranslatef(490+(loop1*40.0f),40.0f,0.0f);			
		glRotatef(-player.spin,0.0f,0.0f,1.0f);				
		glColor3f(0.0f,1.0f,0.0f);							
		glBegin(GL_LINES);									
			glVertex2d(-5,-5);								
			glVertex2d( 5, 5);								
			glVertex2d( 5,-5);								
			glVertex2d(-5, 5);								
		glEnd();											
		glRotatef(-player.spin*0.5f,0.0f,0.0f,1.0f);		
		glColor3f(0.0f,0.75f,0.0f);							
		glBegin(GL_LINES);									
			glVertex2d(-7, 0);								
			glVertex2d( 7, 0);								
			glVertex2d( 0,-7);								
			glVertex2d( 0, 7);								
		glEnd();											
	}

	filled=TRUE;											
	glLineWidth(2.0f);										
	glDisable(GL_LINE_SMOOTH);								
	glLoadIdentity();										
	for (loop1=0; loop1<11; loop1++)						
	{
		for (loop2=0; loop2<11; loop2++)					
		{
			glColor3f(0.0f,0.5f,1.0f);						
			if (hline[loop1][loop2])						
			{
				glColor3f(1.0f,1.0f,1.0f);					
			}

			if (loop1<10)									
			{
				if (!hline[loop1][loop2])					
				{
					filled=FALSE;							
				}
				glBegin(GL_LINES);							
					glVertex2d(20+(loop1*60),70+(loop2*40));
					glVertex2d(80+(loop1*60),70+(loop2*40));
				glEnd();									
			}

			glColor3f(0.0f,0.5f,1.0f);						
			if (vline[loop1][loop2])						
			{
				glColor3f(1.0f,1.0f,1.0f);					
			}
			if (loop2<10)									
			{
				if (!vline[loop1][loop2])					
				{
					filled=FALSE;							
				}
				glBegin(GL_LINES);							
					glVertex2d(20+(loop1*60),70+(loop2*40));
					glVertex2d(20+(loop1*60),110+(loop2*40));
				glEnd();									
			}

			glEnable(GL_TEXTURE_2D);						
			glColor3f(1.0f,1.0f,1.0f);						
			glBindTexture(GL_TEXTURE_2D, texture[1]);		
			if ((loop1<10) && (loop2<10))					
			{
				
				if (hline[loop1][loop2] && hline[loop1][loop2+1] && vline[loop1][loop2] && vline[loop1+1][loop2])
				{
					glBegin(GL_QUADS);						
						glTexCoord2f(float(loop1/10.0f)+0.1f,1.0f-(float(loop2/10.0f)));
						glVertex2d(20+(loop1*60)+59,(70+loop2*40+1));	
						glTexCoord2f(float(loop1/10.0f),1.0f-(float(loop2/10.0f)));
						glVertex2d(20+(loop1*60)+1,(70+loop2*40+1));	
						glTexCoord2f(float(loop1/10.0f),1.0f-(float(loop2/10.0f)+0.1f));
						glVertex2d(20+(loop1*60)+1,(70+loop2*40)+39);	
						glTexCoord2f(float(loop1/10.0f)+0.1f,1.0f-(float(loop2/10.0f)+0.1f));
						glVertex2d(20+(loop1*60)+59,(70+loop2*40)+39);	
					glEnd();								
				}
			}
			glDisable(GL_TEXTURE_2D);						
		}
	}
	glLineWidth(1.0f);										

	if (anti)												
	{
		glEnable(GL_LINE_SMOOTH);							
	}

	if (hourglass.fx==1)									
	{
		glLoadIdentity();									
		glTranslatef(20.0f+(hourglass.x*60),70.0f+(hourglass.y*40),0.0f);	
		glRotatef(hourglass.spin,0.0f,0.0f,1.0f);			
		glColor3ub(rand()%255,rand()%255,rand()%255);		
		glBegin(GL_LINES);									
			glVertex2d(-5,-5);								
			glVertex2d( 5, 5);								
			glVertex2d( 5,-5);								
			glVertex2d(-5, 5);								
			glVertex2d(-5, 5);								
			glVertex2d( 5, 5);								
			glVertex2d(-5,-5);								
			glVertex2d( 5,-5);								
		glEnd();											
	}

	glLoadIdentity();										
	glTranslatef(player.fx+20.0f,player.fy+70.0f,0.0f);		
	glRotatef(player.spin,0.0f,0.0f,1.0f);					
	glColor3f(0.0f,1.0f,0.0f);								
	glBegin(GL_LINES);										
		glVertex2d(-5,-5);									
		glVertex2d( 5, 5);									
		glVertex2d( 5,-5);									
		glVertex2d(-5, 5);									
	glEnd();												
	glRotatef(player.spin*0.5f,0.0f,0.0f,1.0f);				
	glColor3f(0.0f,0.75f,0.0f);								
	glBegin(GL_LINES);										
		glVertex2d(-7, 0);									
		glVertex2d( 7, 0);									
		glVertex2d( 0,-7);									
		glVertex2d( 0, 7);									
	glEnd();												

	for (loop1=0; loop1<(stage*level); loop1++)				
	{
		glLoadIdentity();									
		glTranslatef(enemy[loop1].fx+20.0f,enemy[loop1].fy+70.0f,0.0f);
		glColor3f(1.0f,0.5f,0.5f);							
		glBegin(GL_LINES);									
			glVertex2d( 0,-7);								
			glVertex2d(-7, 0);								
			glVertex2d(-7, 0);								
			glVertex2d( 0, 7);								
			glVertex2d( 0, 7);								
			glVertex2d( 7, 0);								
			glVertex2d( 7, 0);								
			glVertex2d( 0,-7);								
		glEnd();											
		glRotatef(enemy[loop1].spin,0.0f,0.0f,1.0f);		
		glColor3f(1.0f,0.0f,0.0f);							
		glBegin(GL_LINES);									
			glVertex2d(-7,-7);								
			glVertex2d( 7, 7);								
			glVertex2d(-7, 7);								
			glVertex2d( 7,-7);								
		glEnd();											
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

	KillFont();												
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

	
	if (!CreateGLWindow("LR21",640,480,16,fullscreen))
	{
		return 0;												
	}

	ResetObjects();												
	TimerInit();

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
			float start=TimerGetTime();							
			
			
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	
			{
				done=TRUE;										
			}
			else												
			{
				SwapBuffers(hDC);								
			}

			while(TimerGetTime()<start+float(steps[adjust]*2.0f)) {}	

			if (keys[VK_F1])									
			{
				keys[VK_F1]=FALSE;								
				KillGLWindow();									
				fullscreen=!fullscreen;							
				
				if (!CreateGLWindow("LR21",640,480,16,fullscreen))
				{
					return 0;									
				}
			}

			if (keys['A'] && !ap)								
			{
				ap=TRUE;										
				anti=!anti;										
			}
			if (!keys['A'])										
			{
				ap=FALSE;										
			}

			if (!gameover && active)							
			{
				for (loop1=0; loop1<(stage*level); loop1++)		
				{
					if ((enemy[loop1].x<player.x) && (enemy[loop1].fy==enemy[loop1].y*40))
					{
						enemy[loop1].x++;						
					}

					if ((enemy[loop1].x>player.x) && (enemy[loop1].fy==enemy[loop1].y*40))
					{
						enemy[loop1].x--;						
					}

					if ((enemy[loop1].y<player.y) && (enemy[loop1].fx==enemy[loop1].x*60))
					{
						enemy[loop1].y++;						
					}

					if ((enemy[loop1].y>player.y) && (enemy[loop1].fx==enemy[loop1].x*60))
					{
						enemy[loop1].y--;						
					}

					if (delay>(3-level) && (hourglass.fx!=2))	
					{
						delay=0;								
						for (loop2=0; loop2<(stage*level); loop2++)	
						{
							if (enemy[loop2].fx<enemy[loop2].x*60)	
							{
								enemy[loop2].fx+=steps[adjust];	
								enemy[loop2].spin+=steps[adjust];	
							}
							if (enemy[loop2].fx>enemy[loop2].x*60)	
							{
								enemy[loop2].fx-=steps[adjust];	
								enemy[loop2].spin-=steps[adjust];	
							}
							if (enemy[loop2].fy<enemy[loop2].y*40)	
							{
								enemy[loop2].fy+=steps[adjust];	
								enemy[loop2].spin+=steps[adjust];	
							}
							if (enemy[loop2].fy>enemy[loop2].y*40)	
							{
								enemy[loop2].fy-=steps[adjust];	
								enemy[loop2].spin-=steps[adjust];	
							}
						}
					}

					
					if ((enemy[loop1].fx==player.fx) && (enemy[loop1].fy==player.fy))
					{
						lives--;								

						if (lives==0)							
						{
							gameover=TRUE;						
						}

						ResetObjects();							
						PlaySound("Data/Die.wav", NULL, SND_SYNC);	
					}
				}

				if (keys[VK_RIGHT] && (player.x<10) && (player.fx==player.x*60) && (player.fy==player.y*40))
				{
					hline[player.x][player.y]=TRUE;				
					player.x++;									
				}
				if (keys[VK_LEFT] && (player.x>0) && (player.fx==player.x*60) && (player.fy==player.y*40))
				{
					player.x--;									
					hline[player.x][player.y]=TRUE;				
				}
				if (keys[VK_DOWN] && (player.y<10) && (player.fx==player.x*60) && (player.fy==player.y*40))
				{
					vline[player.x][player.y]=TRUE;				
					player.y++;									
				}
				if (keys[VK_UP] && (player.y>0) && (player.fx==player.x*60) && (player.fy==player.y*40))
				{
					player.y--;									
					vline[player.x][player.y]=TRUE;				
				}

				if (player.fx<player.x*60)						
				{
					player.fx+=steps[adjust];					
				}
				if (player.fx>player.x*60)						
				{
					player.fx-=steps[adjust];					
				}
				if (player.fy<player.y*40)						
				{
					player.fy+=steps[adjust];					
				}
				if (player.fy>player.y*40)						
				{
					player.fy-=steps[adjust];					
				}
			}
			else												
			{
				if (keys[' '])									
				{
					gameover=FALSE;								
					filled=TRUE;								
					level=1;									
					level2=1;									
					stage=0;									
					lives=5;									
				}
			}

			if (filled)											
			{
				PlaySound("Data/Complete.wav", NULL, SND_SYNC);	
				stage++;										
				if (stage>3)									
				{
					stage=1;									
					level++;									
					level2++;									
					if (level>3)								
					{
						level=3;								
						lives++;								
						if (lives>5)							
						{
							lives=5;							
						}
					} 
				}

				ResetObjects();									

				for (loop1=0; loop1<11; loop1++)				
				{
					for (loop2=0; loop2<11; loop2++)			
					{
						if (loop1<10)							
						{
							hline[loop1][loop2]=FALSE;			
						}
						if (loop2<10)							
						{
							vline[loop1][loop2]=FALSE;			
						}
					}
				}
			}

			
			if ((player.fx==hourglass.x*60) && (player.fy==hourglass.y*40) && (hourglass.fx==1))
			{
				
				PlaySound("Data/freeze.wav", NULL, SND_ASYNC | SND_LOOP);
				hourglass.fx=2;									
				hourglass.fy=0;									
			}

			player.spin+=0.5f*steps[adjust];					
			if (player.spin>360.0f)								
			{
				player.spin-=360;								
			}

			hourglass.spin-=0.25f*steps[adjust];				
			if (hourglass.spin<0.0f)							
			{
				hourglass.spin+=360.0f;							
			}

			hourglass.fy+=steps[adjust];						
			if ((hourglass.fx==0) && (hourglass.fy>6000/level))	
			{													
				PlaySound("Data/hourglass.wav", NULL, SND_ASYNC);	
				hourglass.x=rand()%10+1;						
				hourglass.y=rand()%11;							
				hourglass.fx=1;									
				hourglass.fy=0;									
			}

			if ((hourglass.fx==1) && (hourglass.fy>6000/level))	
			{													
				hourglass.fx=0;									
				hourglass.fy=0;									
			}

			if ((hourglass.fx==2) && (hourglass.fy>500+(500*level)))	
			{													
				PlaySound(NULL, NULL, 0);						
				hourglass.fx=0;									
				hourglass.fy=0;									
			}

			delay++;											
		}
	}

	
	KillGLWindow();												
	return (msg.wParam);										
}
