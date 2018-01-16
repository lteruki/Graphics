
#include <windows.h>											
#include <gl\gl.h>												
#include <gl\glu.h>												
#include <vfw.h>												
#include "NeHeGL.h"												

#pragma comment( lib, "opengl32.lib" )							
#pragma comment( lib, "glu32.lib" )								
#pragma comment( lib, "vfw32.lib" )								

#ifndef CDS_FULLSCREEN											
#define CDS_FULLSCREEN 4										
#endif															

GL_Window*	g_window;
Keys*		g_keys;


float		angle;												
int			next;												
int			frame=0;											
int			effect;												
bool		sp;													
bool		env=TRUE;											
bool		ep;													
bool		bg=TRUE;											
bool		bp;													

AVISTREAMINFO		psi;										
PAVISTREAM			pavi;										
PGETFRAME			pgf;										
BITMAPINFOHEADER	bmih;										
long				lastframe;									
int					width;										
int					height;										
char				*pdata;										
int					mpf;										

GLUquadricObj *quadratic;										

HDRAWDIB hdd;													
HBITMAP hBitmap;												
HDC hdc = CreateCompatibleDC(0);								
unsigned char* data = 0;										

void flipIt(void* buffer)										
{
	void* b = buffer;											
	__asm														
	{
		mov ecx, 256*256										
		mov ebx, b												
		label:													
			mov al,[ebx+0]										
			mov ah,[ebx+2]										
			mov [ebx+2],al										
			mov [ebx+0],ah										
			
			add ebx,3											
			dec ecx												
			jnz label											
	}
}

void OpenAVI(LPCSTR szFile)										
{
	TCHAR	title[100];											

	AVIFileInit();												

	
	if (AVIStreamOpenFromFile(&pavi, szFile, streamtypeVIDEO, 0, OF_READ, NULL) !=0)
	{
		
		MessageBox (HWND_DESKTOP, "Failed To Open The AVI Stream", "Error", MB_OK | MB_ICONEXCLAMATION);
	}

	AVIStreamInfo(pavi, &psi, sizeof(psi));						
	width=psi.rcFrame.right-psi.rcFrame.left;					
	height=psi.rcFrame.bottom-psi.rcFrame.top;					

	lastframe=AVIStreamLength(pavi);							

	mpf=AVIStreamSampleToTime(pavi,lastframe)/lastframe;		

	bmih.biSize = sizeof (BITMAPINFOHEADER);					
	bmih.biPlanes = 1;											
	bmih.biBitCount = 24;										
	bmih.biWidth = 256;											
	bmih.biHeight = 256;										
	bmih.biCompression = BI_RGB;								

	hBitmap = CreateDIBSection (hdc, (BITMAPINFO*)(&bmih), DIB_RGB_COLORS, (void**)(&data), NULL, NULL);
	SelectObject (hdc, hBitmap);								

	pgf=AVIStreamGetFrameOpen(pavi, NULL);						
	if (pgf==NULL)
	{
		
		MessageBox (HWND_DESKTOP, "Failed To Open The AVI Frame", "Error", MB_OK | MB_ICONEXCLAMATION);
	}

	
	wsprintf (title, "LR35 player: Width: %d, Height: %d, Frames: %d", width, height, lastframe);
	SetWindowText(g_window->hWnd, title);						
}

void GrabAVIFrame(int frame)									
{
	LPBITMAPINFOHEADER lpbi;									
	lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, frame);	
	pdata=(char *)lpbi+lpbi->biSize+lpbi->biClrUsed * sizeof(RGBQUAD);	

	
	DrawDibDraw (hdd, hdc, 0, 0, 256, 256, lpbi, pdata, 0, 0, width, height, 0);

	flipIt(data);												

	
	glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void CloseAVI(void)												
{
	DeleteObject(hBitmap);										
	DrawDibClose(hdd);											
	AVIStreamGetFrameClose(pgf);								
	AVIStreamRelease(pavi);										
	AVIFileExit();												
}

BOOL Initialize (GL_Window* window, Keys* keys)					
{
	g_window	= window;
	g_keys		= keys;

	
	angle		= 0.0f;											
	hdd = DrawDibOpen();										
	glClearColor (0.0f, 0.0f, 0.0f, 0.5f);						
	glClearDepth (1.0f);										
	glDepthFunc (GL_LEQUAL);									
	glEnable(GL_DEPTH_TEST);									
	glShadeModel (GL_SMOOTH);									
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			

	quadratic=gluNewQuadric();									
	gluQuadricNormals(quadratic, GLU_SMOOTH);					
	gluQuadricTexture(quadratic, GL_TRUE);						

	glEnable(GL_TEXTURE_2D);									
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);		
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);		

	OpenAVI("data/face2.avi");									

	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	return TRUE;												
}

void Deinitialize (void)										
{
	CloseAVI();													
}

void Update (DWORD milliseconds)								
{
	if (g_keys->keyDown [VK_ESCAPE] == TRUE)					
	{
		TerminateApplication (g_window);						
	}

	if (g_keys->keyDown [VK_F1] == TRUE)						
	{
		ToggleFullscreen (g_window);							
	}

	if ((g_keys->keyDown [' ']) && !sp)							
	{
		sp=TRUE;												
		effect++;												
		if (effect>3)											
			effect=0;											
	}

	if (!g_keys->keyDown[' '])									
		sp=FALSE;												

	if ((g_keys->keyDown ['B']) && !bp)							
	{
		bp=TRUE;												
		bg=!bg;													
	}

	if (!g_keys->keyDown['B'])									
		bp=FALSE;												

	if ((g_keys->keyDown ['E']) && !ep)							
	{
		ep=TRUE;												
		env=!env;												
	}

	if (!g_keys->keyDown['E'])									
		ep=FALSE;												

	angle += (float)(milliseconds) / 60.0f;						

	next+=milliseconds;											
	frame=next/mpf;												

	if (frame>=lastframe)										
	{
		frame=0;												
		next=0;													
	}
}

void Draw (void)												
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

	GrabAVIFrame(frame);										

	if (bg)														
	{
		glLoadIdentity();										
		glBegin(GL_QUADS);										
			
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 11.0f,  8.3f, -20.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-11.0f,  8.3f, -20.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-11.0f, -8.3f, -20.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 11.0f, -8.3f, -20.0f);
		glEnd();												
	}

	glLoadIdentity ();											
	glTranslatef (0.0f, 0.0f, -10.0f);							

	if (env)													
	{
		glEnable(GL_TEXTURE_GEN_S);								
		glEnable(GL_TEXTURE_GEN_T);								
	}
	
	glRotatef(angle*2.3f,1.0f,0.0f,0.0f);						
	glRotatef(angle*1.8f,0.0f,1.0f,0.0f);						
	glTranslatef(0.0f,0.0f,2.0f);								

	switch (effect)												
	{
	case 0:														
		glRotatef (angle*1.3f, 1.0f, 0.0f, 0.0f);				
		glRotatef (angle*1.1f, 0.0f, 1.0f, 0.0f);				
		glRotatef (angle*1.2f, 0.0f, 0.0f, 1.0f);				
		glBegin(GL_QUADS);										
			
			glNormal3f( 0.0f, 0.0f, 0.5f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
			
			glNormal3f( 0.0f, 0.0f,-0.5f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
			
			glNormal3f( 0.0f, 0.5f, 0.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
			
			glNormal3f( 0.0f,-0.5f, 0.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
			
			glNormal3f( 0.5f, 0.0f, 0.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
			
			glNormal3f(-0.5f, 0.0f, 0.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
			glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
		glEnd();												
		break;													

	case 1:														
		glRotatef (angle*1.3f, 1.0f, 0.0f, 0.0f);				
		glRotatef (angle*1.1f, 0.0f, 1.0f, 0.0f);				
		glRotatef (angle*1.2f, 0.0f, 0.0f, 1.0f);				
		gluSphere(quadratic,1.3f,20,20);						
		break;													

	case 2:														
		glRotatef (angle*1.3f, 1.0f, 0.0f, 0.0f);				
		glRotatef (angle*1.1f, 0.0f, 1.0f, 0.0f);				
		glRotatef (angle*1.2f, 0.0f, 0.0f, 1.0f);				
		glTranslatef(0.0f,0.0f,-1.5f);							
		gluCylinder(quadratic,1.0f,1.0f,3.0f,32,32);			
		break;													
	}
	
	if (env)													
	{
		glDisable(GL_TEXTURE_GEN_S);							
		glDisable(GL_TEXTURE_GEN_T);							
	}
	
	glFlush ();													
}
