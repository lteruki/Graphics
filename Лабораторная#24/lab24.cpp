
#include	<windows.h>										
#include	<stdio.h>										
#include	<stdarg.h>										
#include	<string.h>										
#include	<gl\gl.h>										
#include	<gl\glu.h>										

HDC			hDC=NULL;										
HGLRC		hRC=NULL;										
HWND		hWnd=NULL;										
HINSTANCE	hInstance;										

bool		keys[256];										
bool		active=TRUE;									
bool		fullscreen=TRUE;								

int			scroll;											
int			maxtokens;										
int			swidth;											
int			sheight;										

GLuint		base;											

typedef struct												
{
	GLubyte	*imageData;										
	GLuint	bpp;											
	GLuint	width;											
	GLuint	height;											
	GLuint	texID;											
} TextureImage;												

TextureImage textures[1];									

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);		

bool LoadTGA(TextureImage *texture, char *filename)			
{    
	GLubyte		TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};	
	GLubyte		TGAcompare[12];								
	GLubyte		header[6];									
	GLuint		bytesPerPixel;								
	GLuint		imageSize;									
	GLuint		temp;										
	GLuint		type=GL_RGBA;								

	FILE *file = fopen(filename, "rb");						

	if(	file==NULL ||										
		fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||	
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0				||	
		fread(header,1,sizeof(header),file)!=sizeof(header))				
	{
		if (file == NULL)									
			return false;									
		else
		{
			fclose(file);									
			return false;									
		}
	}

	texture->width  = header[1] * 256 + header[0];			
	texture->height = header[3] * 256 + header[2];			
    
 	if(	texture->width	<=0	||								
		texture->height	<=0	||								
		(header[4]!=24 && header[4]!=32))					
	{
		fclose(file);										
		return false;										
	}

	texture->bpp	= header[4];							
	bytesPerPixel	= texture->bpp/8;						
	imageSize		= texture->width*texture->height*bytesPerPixel;	

	texture->imageData=(GLubyte *)malloc(imageSize);		

	if(	texture->imageData==NULL ||							
		fread(texture->imageData, 1, imageSize, file)!=imageSize)	
	{
		if(texture->imageData!=NULL)						
			free(texture->imageData);						

		fclose(file);										
		return false;										
	}

	for(GLuint i=0; i<int(imageSize); i+=bytesPerPixel)		
	{														
		temp=texture->imageData[i];							
		texture->imageData[i] = texture->imageData[i + 2];	
		texture->imageData[i + 2] = temp;					
	}

	fclose (file);											

	
	glGenTextures(1, &texture[0].texID);					

	glBindTexture(GL_TEXTURE_2D, texture[0].texID);			
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
	
	if (texture[0].bpp==24)									
	{
		type=GL_RGB;										
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);

	return true;											
}

GLvoid BuildFont(GLvoid)									
{
	base=glGenLists(256);									
	glBindTexture(GL_TEXTURE_2D, textures[0].texID);		
	for (int loop1=0; loop1<256; loop1++)					
	{
		float cx=float(loop1%16)/16.0f;						
		float cy=float(loop1/16)/16.0f;						

		glNewList(base+loop1,GL_COMPILE);					
			glBegin(GL_QUADS);								
				glTexCoord2f(cx,1.0f-cy-0.0625f);			
				glVertex2d(0,16);							
				glTexCoord2f(cx+0.0625f,1.0f-cy-0.0625f);	
				glVertex2i(16,16);							
				glTexCoord2f(cx+0.0625f,1.0f-cy-0.001f);	
				glVertex2i(16,0);							
				glTexCoord2f(cx,1.0f-cy-0.001f);			
				glVertex2i(0,0);							
			glEnd();										
			glTranslated(14,0,0);							
		glEndList();										
	}														
}

GLvoid KillFont(GLvoid)										
{
	glDeleteLists(base,256);								
}

GLvoid glPrint(GLint x, GLint y, int set, const char *fmt, ...)	
{
	char		text[1024];									
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

	glScalef(1.0f,2.0f,1.0f);								

	glCallLists(strlen(text),GL_UNSIGNED_BYTE, text);		
	glDisable(GL_TEXTURE_2D);								
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)			
{
	swidth=width;											
	sheight=height;											
	if (height==0)											
	{
		height=1;											
	}
	glViewport(0,0,width,height);							
	glMatrixMode(GL_PROJECTION);							
	glLoadIdentity();										
	glOrtho(0.0f,640,480,0.0f,-1.0f,1.0f);					
	glMatrixMode(GL_MODELVIEW);								
	glLoadIdentity();										
}

int InitGL(GLvoid)											
{
	if (!LoadTGA(&textures[0],"Data/Font.TGA"))				
	{
		return false;										
	}

	BuildFont();											

	glShadeModel(GL_SMOOTH);								
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);					
	glClearDepth(1.0f);										
	glBindTexture(GL_TEXTURE_2D, textures[0].texID);		

	return TRUE;											
}

int DrawGLScene(GLvoid)										
{
	char	*token;											
	int		cnt=0;											

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

	glColor3f(1.0f,0.5f,0.5f);								
	glPrint(50,16,1,"Renderer");							
	glPrint(80,48,1,"Vendor");								
	glPrint(66,80,1,"Version");								

	glColor3f(1.0f,0.7f,0.4f);								
	glPrint(200,16,1,(char *)glGetString(GL_RENDERER));		
	glPrint(200,48,1,(char *)glGetString(GL_VENDOR));		
	glPrint(200,80,1,(char *)glGetString(GL_VERSION));		

	glColor3f(0.5f,0.5f,1.0f);								
	glPrint(192,432,1,"LR24");					

	glLoadIdentity();										
	glColor3f(1.0f,1.0f,1.0f);								
	glBegin(GL_LINE_STRIP);									
		glVertex2d(639,417);								
		glVertex2d(  0,417);								
		glVertex2d(  0,480);								
		glVertex2d(639,480);								
		glVertex2d(639,128);								
	glEnd();												
	glBegin(GL_LINE_STRIP);									
		glVertex2d(  0,128);								
		glVertex2d(639,128);								
		glVertex2d(639,  1);								
		glVertex2d(  0,  1);								
		glVertex2d(  0,417);								
	glEnd();												

	glScissor(1	,int(0.135416f*sheight),swidth-2,int(0.597916f*sheight));	
	glEnable(GL_SCISSOR_TEST);								

	char* text=(char *)malloc(strlen((char *)glGetString(GL_EXTENSIONS))+1);	
	strcpy (text,(char *)glGetString(GL_EXTENSIONS));		

	token=strtok(text," ");									
	while(token!=NULL)										
	{
		cnt++;												
		if (cnt>maxtokens)									
		{
			maxtokens=cnt;									
		}

		glColor3f(0.5f,1.0f,0.5f);							
		glPrint(0,96+(cnt*32)-scroll,0,"%i",cnt);			
		glColor3f(1.0f,1.0f,0.5f);							
		glPrint(50,96+(cnt*32)-scroll,0,token);				
		token=strtok(NULL," ");								
	}

	glDisable(GL_SCISSOR_TEST);								

	free(text);												

	glFlush();												
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
			
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","LR24 GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
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
		fullscreen=TRUE;
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

	
	if (!CreateGLWindow("LR24",640,480,16,fullscreen))
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

				if (keys[VK_F1])								
				{
					keys[VK_F1]=FALSE;							
					KillGLWindow();								
					fullscreen=!fullscreen;						
					
					if (!CreateGLWindow("LR24",640,480,16,fullscreen))
					{
						return 0;								
					}
				}

				if (keys[VK_UP] && (scroll>0))					
				{
					scroll-=2;									
				}

				if (keys[VK_DOWN] && (scroll<32*(maxtokens-9)))	
				{
					scroll+=2;									
				}
			}
		}
	}

	
	KillGLWindow();												
	return (msg.wParam);										
}
