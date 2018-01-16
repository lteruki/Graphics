
#include <windows.h>		
#include <math.h>			
#include <stdio.h>			
#include <gl\gl.h>			
#include <gl\glu.h>			
#include <gl\glaux.h>		

HDC			hDC=NULL;		
HGLRC		hRC=NULL;		
HWND		hWnd=NULL;		
HINSTANCE	hInstance;		

bool	keys[256];			
bool	active=TRUE;		
bool	fullscreen=TRUE;	

GLuint	base;				
GLuint	texture[2];			
GLuint	loop;				

GLfloat	cnt1;				
GLfloat	cnt2;				

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
        AUX_RGBImageRec *TextureImage[2];               
        memset(TextureImage,0,sizeof(void *)*2);        

        if ((TextureImage[0]=LoadBMP("Data/Font.bmp")) &&
			(TextureImage[1]=LoadBMP("Data/Bumps.bmp")))
        {
                Status=TRUE;                            
                glGenTextures(2, &texture[0]);          

				for (loop=0; loop<2; loop++)
				{
	                glBindTexture(GL_TEXTURE_2D, texture[loop]);
			        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
					glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[loop]->sizeX, TextureImage[loop]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[loop]->data);
				}
        }
		for (loop=0; loop<2; loop++)
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
        return Status;                                  
}

GLvoid BuildFont(GLvoid)								
{
	float	cx;											
	float	cy;											

	base=glGenLists(256);								
	glBindTexture(GL_TEXTURE_2D, texture[0]);			
	for (loop=0; loop<256; loop++)						
	{
		cx=float(loop%16)/16.0f;						
		cy=float(loop/16)/16.0f;						

		glNewList(base+loop,GL_COMPILE);				
			glBegin(GL_QUADS);							
				glTexCoord2f(cx,1-cy-0.0625f);			
				glVertex2i(0,0);						
				glTexCoord2f(cx+0.0625f,1-cy-0.0625f);	
				glVertex2i(16,0);						
				glTexCoord2f(cx+0.0625f,1-cy);			
				glVertex2i(16,16);						
				glTexCoord2f(cx,1-cy);					
				glVertex2i(0,16);						
			glEnd();									
			glTranslated(10,0,0);						
		glEndList();									
	}													
}

GLvoid KillFont(GLvoid)									
{
	glDeleteLists(base,256);							
}

GLvoid glPrint(GLint x, GLint y, char *string, int set)	
{
	if (set>1)
	{
		set=1;
	}
	glBindTexture(GL_TEXTURE_2D, texture[0]);			
	glDisable(GL_DEPTH_TEST);							
	glMatrixMode(GL_PROJECTION);						
	glPushMatrix();										
	glLoadIdentity();									
	glOrtho(0,640,0,480,-1,1);							
	glMatrixMode(GL_MODELVIEW);							
	glPushMatrix();										
	glLoadIdentity();									
	glTranslated(x,y,0);								
	glListBase(base-32+(128*set));						
	glCallLists(strlen(string),GL_UNSIGNED_BYTE,string);
	glMatrixMode(GL_PROJECTION);						
	glPopMatrix();										
	glMatrixMode(GL_MODELVIEW);							
	glPopMatrix();										
	glEnable(GL_DEPTH_TEST);							
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
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);	
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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				
	glClearDepth(1.0);									
	glDepthFunc(GL_LEQUAL);								
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);					
	glShadeModel(GL_SMOOTH);							
	glEnable(GL_TEXTURE_2D);							
	return TRUE;										
}

int DrawGLScene(GLvoid)									
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glLoadIdentity();									
	glBindTexture(GL_TEXTURE_2D, texture[1]);			
	glTranslatef(0.0f,0.0f,-5.0f);						
	glRotatef(45.0f,0.0f,0.0f,1.0f);					
	glRotatef(cnt1*30.0f,1.0f,1.0f,0.0f);				
	glDisable(GL_BLEND);								
	glColor3f(1.0f,1.0f,1.0f);							
	glBegin(GL_QUADS);									
		glTexCoord2d(0.0f,0.0f);						
		glVertex2f(-1.0f, 1.0f);						
		glTexCoord2d(1.0f,0.0f);						
		glVertex2f( 1.0f, 1.0f);						
		glTexCoord2d(1.0f,1.0f);						
		glVertex2f( 1.0f,-1.0f);						
		glTexCoord2d(0.0f,1.0f);						
		glVertex2f(-1.0f,-1.0f);						
	glEnd();											
	glRotatef(90.0f,1.0f,1.0f,0.0f);					
	glBegin(GL_QUADS);									
		glTexCoord2d(0.0f,0.0f);						
		glVertex2f(-1.0f, 1.0f);						
		glTexCoord2d(1.0f,0.0f);						
		glVertex2f( 1.0f, 1.0f);						
		glTexCoord2d(1.0f,1.0f);						
		glVertex2f( 1.0f,-1.0f);						
		glTexCoord2d(0.0f,1.0f);						
		glVertex2f(-1.0f,-1.0f);						
	glEnd();											
	glEnable(GL_BLEND);									

	glLoadIdentity();									
	
	glColor3f(1.0f*float(cos(cnt1)),1.0f*float(sin(cnt2)),1.0f-0.5f*float(cos(cnt1+cnt2)));
	glPrint(int((280+250*cos(cnt1))),int(235+200*sin(cnt2)),"LR17",0);		

	glColor3f(1.0f*float(sin(cnt2)),1.0f-0.5f*float(cos(cnt1+cnt2)),1.0f*float(cos(cnt1)));
	glPrint(int((280+230*cos(cnt2))),int(235+200*sin(cnt1)),"OpenGL",1);	

	glColor3f(0.0f,0.0f,1.0f);							
	glPrint(int(240+200*cos((cnt2+cnt1)/5)),2,"Aleksandr",0);

	glColor3f(1.0f,1.0f,1.0f);							
	glPrint(int(242+200*cos((cnt2+cnt1)/5)),2,"Aleksandr",0);

	cnt1+=0.01f;										
	cnt2+=0.0081f;										
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

	
	if (!CreateGLWindow("LR17",640,480,16,fullscreen))
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
			
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	
			{
				done=TRUE;							
			}
			else									
			{
				SwapBuffers(hDC);					
			}

			if (keys[VK_F1])						
			{
				keys[VK_F1]=FALSE;					
				KillGLWindow();						
				fullscreen=!fullscreen;				
				
				if (!CreateGLWindow("NeHe & Giuseppe D'Agata's 2D Font Tutorial",640,480,16,fullscreen))
				{
					return 0;						
				}
			}
		}
	}

	
	KillGLWindow();									
	return (msg.wParam);							
}
