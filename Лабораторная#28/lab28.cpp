
#include <windows.h>		
#include <math.h>			
#include <stdio.h>			
#include <stdlib.h>			
#include <gl\gl.h>			
#include <gl\glu.h>			
#include <gl\glaux.h>		


typedef struct point_3d {			
	double x, y, z;
} POINT_3D;

typedef struct bpatch {				
	POINT_3D	anchors[4][4];			
	GLuint		dlBPatch;				
	GLuint		texture;				
} BEZIER_PATCH;

HDC				hDC=NULL;			
HGLRC			hRC=NULL;			
HWND			hWnd=NULL;			
HINSTANCE		hInstance;			

DEVMODE			DMsaved;			

bool			keys[256];			
bool			active=TRUE;		
bool			fullscreen=TRUE;	

GLfloat			rotz = 0.0f;		
BEZIER_PATCH	mybezier;			
BOOL			showCPoints=TRUE;	
int				divs = 7;			


LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	




POINT_3D pointAdd(POINT_3D p, POINT_3D q) {
	p.x += q.x;		p.y += q.y;		p.z += q.z;
	return p;
}


POINT_3D pointTimes(double c, POINT_3D p) {
	p.x *= c;	p.y *= c;	p.z *= c;
	return p;
}


POINT_3D makePoint(double a, double b, double c) {
	POINT_3D p;
	p.x = a;	p.y = b;	p.z = c;
	return p;
}




POINT_3D Bernstein(float u, POINT_3D *p) {
	POINT_3D	a, b, c, d, r;

	a = pointTimes(pow(u,3), p[0]);
	b = pointTimes(3*pow(u,2)*(1-u), p[1]);
	c = pointTimes(3*u*pow((1-u),2), p[2]);
	d = pointTimes(pow((1-u),3), p[3]);

	r = pointAdd(pointAdd(a, b), pointAdd(c, d));

	return r;
}



GLuint genBezier(BEZIER_PATCH patch, int divs) {
	int			u = 0, v;
	float		py, px, pyold; 
	GLuint		drawlist = glGenLists(1);		
	POINT_3D	temp[4];
	POINT_3D	*last = (POINT_3D*)malloc(sizeof(POINT_3D)*(divs+1));
												

	if (patch.dlBPatch != NULL)					
		glDeleteLists(patch.dlBPatch, 1);

	temp[0] = patch.anchors[0][3];				
	temp[1] = patch.anchors[1][3];
	temp[2] = patch.anchors[2][3];
	temp[3] = patch.anchors[3][3];

	for (v=0;v<=divs;v++) {						
		px = ((float)v)/((float)divs);			
	
		last[v] = Bernstein(px, temp);
	}

	glNewList(drawlist, GL_COMPILE);				
	glBindTexture(GL_TEXTURE_2D, patch.texture);	

	for (u=1;u<=divs;u++) {
		py	  = ((float)u)/((float)divs);			
		pyold = ((float)u-1.0f)/((float)divs);		

		temp[0] = Bernstein(py, patch.anchors[0]);	
		temp[1] = Bernstein(py, patch.anchors[1]);
		temp[2] = Bernstein(py, patch.anchors[2]);
		temp[3] = Bernstein(py, patch.anchors[3]);

		glBegin(GL_TRIANGLE_STRIP);					

		for (v=0;v<=divs;v++) {
			px = ((float)v)/((float)divs);			

			glTexCoord2f(pyold, px);				
			glVertex3d(last[v].x, last[v].y, last[v].z);	

			last[v] = Bernstein(px, temp);			
			glTexCoord2f(py, px);					
			glVertex3d(last[v].x, last[v].y, last[v].z);	
		}

		glEnd();									
	}
	
	glEndList();								

	free(last);									
	return drawlist;							
}



void initBezier(void) {	
	mybezier.anchors[0][0] = makePoint(-0.75,	-0.75,	-0.5);	
	mybezier.anchors[0][1] = makePoint(-0.25,	-0.75,	0.0);
	mybezier.anchors[0][2] = makePoint(0.25,	-0.75,	0.0);
	mybezier.anchors[0][3] = makePoint(0.75,	-0.75,	-0.5);
	mybezier.anchors[1][0] = makePoint(-0.75,	-0.25,	-0.75);
	mybezier.anchors[1][1] = makePoint(-0.25,	-0.25,	0.5);
	mybezier.anchors[1][2] = makePoint(0.25,	-0.25,	0.5);
	mybezier.anchors[1][3] = makePoint(0.75,	-0.25,	-0.75);
	mybezier.anchors[2][0] = makePoint(-0.75,	0.25,	0.0);
	mybezier.anchors[2][1] = makePoint(-0.25,	0.25,	-0.5);
	mybezier.anchors[2][2] = makePoint(0.25,	0.25,	-0.5);
	mybezier.anchors[2][3] = makePoint(0.75,	0.25,	0.0);
	mybezier.anchors[3][0] = makePoint(-0.75,	0.75,	-0.5);
	mybezier.anchors[3][1] = makePoint(-0.25,	0.75,	-1.0);
	mybezier.anchors[3][2] = makePoint(0.25,	0.75,	-1.0);
	mybezier.anchors[3][3] = makePoint(0.75,	0.75,	-0.5);
	mybezier.dlBPatch = NULL;
}




BOOL LoadGLTexture(GLuint *texPntr, char* name)
{
	BOOL success = FALSE;
	AUX_RGBImageRec *TextureImage = NULL;

	glGenTextures(1, texPntr);						

	FILE* test=NULL;
	TextureImage = NULL;

	test = fopen(name, "r");						
	if (test != NULL) {								
		fclose(test);									
		TextureImage = auxDIBImageLoad(name);			
	}

	if (TextureImage != NULL) {						
		success = TRUE;

		
		glBindTexture(GL_TEXTURE_2D, *texPntr);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage->sizeX, TextureImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}

	if (TextureImage->data)
		free(TextureImage->data);

	return success;
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
	glEnable(GL_TEXTURE_2D);							
	glShadeModel(GL_SMOOTH);							
	glClearColor(0.05f, 0.05f, 0.05f, 0.5f);			
	glClearDepth(1.0f);									
	glEnable(GL_DEPTH_TEST);							
	glDepthFunc(GL_LEQUAL);								
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	

	initBezier();											
	LoadGLTexture(&(mybezier.texture), "./data/NeHe.bmp");	
	mybezier.dlBPatch = genBezier(mybezier, divs);			

	return TRUE;										
}



int DrawGLScene(GLvoid)	{								
	int i, j;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glLoadIdentity();									
	glTranslatef(0.0f,0.0f,-4.0f);						
	glRotatef(-75.0f,1.0f,0.0f,0.0f);
	glRotatef(rotz,0.0f,0.0f,1.0f);						
		
	glCallList(mybezier.dlBPatch);						
														

	if (showCPoints) {									
		glDisable(GL_TEXTURE_2D);
		glColor3f(1.0f,0.0f,0.0f);
		for(i=0;i<4;i++) {								
			glBegin(GL_LINE_STRIP);
			for(j=0;j<4;j++)
				glVertex3d(mybezier.anchors[i][j].x, mybezier.anchors[i][j].y, mybezier.anchors[i][j].z);
			glEnd();
		}
		for(i=0;i<4;i++) {								
			glBegin(GL_LINE_STRIP);
			for(j=0;j<4;j++)
				glVertex3d(mybezier.anchors[j][i].x, mybezier.anchors[j][i].y, mybezier.anchors[j][i].z);
			glEnd();
		}
		glColor3f(1.0f,1.0f,1.0f);
		glEnable(GL_TEXTURE_2D);
	}

	return TRUE;										
}


GLvoid KillGLWindow(GLvoid)								
{
	if (fullscreen)										
	{
		if (!ChangeDisplaySettings(NULL,CDS_TEST)) { 
			ChangeDisplaySettings(NULL,CDS_RESET);		
			ChangeDisplaySettings(&DMsaved,CDS_RESET);	
		} else {
			ChangeDisplaySettings(NULL,CDS_RESET);
		}
			
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
	
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DMsaved); 

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

	if (!RegisterClass(&wc))									
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											
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

	
	if (!CreateGLWindow("LR28",640,480,16,fullscreen))
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


			if (keys[VK_LEFT])	rotz -= 0.8f;		
			if (keys[VK_RIGHT])	rotz += 0.8f;		
			if (keys[VK_UP]) {						
				divs++;
				mybezier.dlBPatch = genBezier(mybezier, divs);	
				keys[VK_UP] = FALSE;
			}
			if (keys[VK_DOWN] && divs > 1) {
				divs--;
				mybezier.dlBPatch = genBezier(mybezier, divs);	
				keys[VK_DOWN] = FALSE;
			}
			if (keys[VK_SPACE]) {					
				showCPoints = !showCPoints;
				keys[VK_SPACE] = FALSE;
			}


			if (keys[VK_F1])						
			{
				keys[VK_F1]=FALSE;					
				KillGLWindow();						
				fullscreen=!fullscreen;				
				
				if (!CreateGLWindow("LR28",640,480,16,fullscreen))
				{
					return 0;						
				}
			}
		}
	}

	
	KillGLWindow();									
	return (msg.wParam);							
}
