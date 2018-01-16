
#include <windows.h>											
#include <stdio.h>												
#include <stdarg.h>												
#include <gl\gl.h>												
#include <gl\glu.h>												
#include <time.h>												
#include "NeHeGL.h"												

#pragma comment( lib, "opengl32.lib" )							
#pragma comment( lib, "glu32.lib" )								
#pragma comment( lib, "winmm.lib" )								

#ifndef		CDS_FULLSCREEN										
#define		CDS_FULLSCREEN 4									
#endif															

void DrawTargets();												

GL_Window*	g_window;
Keys*		g_keys;


GLuint		base;												
GLfloat		roll;												
GLint		level=1;											
GLint		miss;												
GLint		kills;												
GLint		score;												
bool		game;												

typedef int (*compfn)(const void*, const void*);				

struct objects {
	GLuint	rot;												
	bool	hit;												
	GLuint	frame;												
	GLuint	dir;												
	GLuint	texid;												
	GLfloat	x;													
	GLfloat y;													
	GLfloat	spin;												
	GLfloat	distance;											
};

typedef struct													
{
	GLubyte	*imageData;											
	GLuint	bpp;												
	GLuint	width;												
	GLuint	height;												
	GLuint	texID;												
} TextureImage;													

TextureImage textures[10];										

objects	object[30];												

struct dimensions {												
	GLfloat	w;													
	GLfloat h;													
};


dimensions size[5] = { {1.0f,1.0f}, {1.0f,1.0f}, {1.0f,1.0f}, {0.5f,1.0f}, {0.75f,1.5f} };

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
			return FALSE;										
		else													
		{
			fclose(file);										
			return FALSE;										
		}
	}

	texture->width  = header[1] * 256 + header[0];				
	texture->height = header[3] * 256 + header[2];				
    
 	if(	texture->width	<=0	||									
		texture->height	<=0	||									
		(header[4]!=24 && header[4]!=32))						
	{
		fclose(file);											
		return FALSE;											
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
		return FALSE;											
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
	base=glGenLists(95);										
	glBindTexture(GL_TEXTURE_2D, textures[9].texID);			
	for (int loop=0; loop<95; loop++)							
	{
		float cx=float(loop%16)/16.0f;							
		float cy=float(loop/16)/8.0f;							

		glNewList(base+loop,GL_COMPILE);						
			glBegin(GL_QUADS);									
				glTexCoord2f(cx,         1.0f-cy-0.120f); glVertex2i(0,0);	
				glTexCoord2f(cx+0.0625f, 1.0f-cy-0.120f); glVertex2i(16,0);	
				glTexCoord2f(cx+0.0625f, 1.0f-cy);		  glVertex2i(16,16);
				glTexCoord2f(cx,         1.0f-cy);		  glVertex2i(0,16);	
			glEnd();											
			glTranslated(10,0,0);								
		glEndList();											
	}															
}

GLvoid glPrint(GLint x, GLint y, const char *string, ...)		
{
	char		text[256];										
	va_list		ap;												

	if (string == NULL)											
		return;													

	va_start(ap, string);										
	    vsprintf(text, string, ap);								
	va_end(ap);													

	glBindTexture(GL_TEXTURE_2D, textures[9].texID);			
	glPushMatrix();												
	glLoadIdentity();											
	glTranslated(x,y,0);										
	glListBase(base-32);										
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);			
	glPopMatrix();												
}

int Compare(struct objects *elem1, struct objects *elem2)		
{
   if ( elem1->distance < elem2->distance)						
      return -1;												
   else if (elem1->distance > elem2->distance)					
      return 1;													
   else															
      return 0;													
}

GLvoid InitObject(int num)										
{
	object[num].rot=1;											
	object[num].frame=0;										
	object[num].hit=FALSE;										
	object[num].texid=rand()%5;									
	object[num].distance=-(float(rand()%4001)/100.0f);			
	object[num].y=-1.5f+(float(rand()%451)/100.0f);				
	
	object[num].x=((object[num].distance-15.0f)/2.0f)-(5*level)-float(rand()%(5*level));
	object[num].dir=(rand()%2);									

	if (object[num].dir==0)										
	{
		object[num].rot=2;										
		object[num].x=-object[num].x;							
	}

	if (object[num].texid==0)									
		object[num].y=-2.0f;									

	if (object[num].texid==1)									
	{
		object[num].dir=3;										
		object[num].x=float(rand()%int(object[num].distance-10.0f))+((object[num].distance-10.0f)/2.0f);
		object[num].y=4.5f;										
	}

	if (object[num].texid==2)									
	{
		object[num].dir=2;										
		object[num].x=float(rand()%int(object[num].distance-10.0f))+((object[num].distance-10.0f)/2.0f);
		object[num].y=-3.0f-float(rand()%(5*level));			
	}

	
	
	
	
	qsort((void *) &object, level, sizeof(struct objects), (compfn)Compare );
}

BOOL Initialize (GL_Window* window, Keys* keys)					
{
	g_window	= window;
	g_keys		= keys;

	srand( (unsigned)time( NULL ) );							

	if ((!LoadTGA(&textures[0],"Data/BlueFace.tga")) ||			
		(!LoadTGA(&textures[1],"Data/Bucket.tga")) ||			
		(!LoadTGA(&textures[2],"Data/Target.tga")) ||			
		(!LoadTGA(&textures[3],"Data/Coke.tga")) ||				
		(!LoadTGA(&textures[4],"Data/Vase.tga")) ||				
		(!LoadTGA(&textures[5],"Data/Explode.tga")) ||			
		(!LoadTGA(&textures[6],"Data/Ground.tga")) ||			
		(!LoadTGA(&textures[7],"Data/Sky.tga")) ||				
		(!LoadTGA(&textures[8],"Data/Crosshair.tga")) ||		
		(!LoadTGA(&textures[9],"Data/Font.tga")))				
	{
		return FALSE;											
	}

	BuildFont();												

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);						
	glClearDepth(1.0f);											
	glDepthFunc(GL_LEQUAL);										
	glEnable(GL_DEPTH_TEST);									
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);			
	glEnable(GL_BLEND);											


	glEnable(GL_TEXTURE_2D);									
	glEnable(GL_CULL_FACE);										

	for (int loop=0; loop<30; loop++)							
		InitObject(loop);										
	
	return TRUE;												
}

void Deinitialize (void)										
{
	glDeleteLists(base,95);										
}

void Selection(void)											
{
	GLuint	buffer[512];										
	GLint	hits;												

	if (game)													
		return;													
	
	PlaySound("data/shot.wav",NULL,SND_ASYNC);					

	
	GLint	viewport[4];

	
	glGetIntegerv(GL_VIEWPORT, viewport);
	glSelectBuffer(512, buffer);								

	
	(void) glRenderMode(GL_SELECT);

	glInitNames();												
	glPushName(0);												

	glMatrixMode(GL_PROJECTION);								
	glPushMatrix();												
	glLoadIdentity();											

	
	gluPickMatrix((GLdouble) mouse_x, (GLdouble) (viewport[3]-mouse_y), 1.0f, 1.0f, viewport);

	
	gluPerspective(45.0f, (GLfloat) (viewport[2]-viewport[0])/(GLfloat) (viewport[3]-viewport[1]), 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);									
	DrawTargets();												
	glMatrixMode(GL_PROJECTION);								
	glPopMatrix();												
	glMatrixMode(GL_MODELVIEW);									
	hits=glRenderMode(GL_RENDER);								
																
	if (hits > 0)												
	{
		int	choose = buffer[3];									
		int depth = buffer[1];									

		for (int loop = 1; loop < hits; loop++)					
		{
			
			if (buffer[loop*4+1] < GLuint(depth))
			{
				choose = buffer[loop*4+3];						
				depth = buffer[loop*4+1];						
			}       
		}

		if (!object[choose].hit)								
		{
			object[choose].hit=TRUE;							
			score+=1;											
			kills+=1;											
			if (kills>level*5)									
			{
				miss=0;											
				kills=0;										
				level+=1;										
				if (level>30)									
					level=30;									
			}
		}
    }
}

void Update(DWORD milliseconds)									
{
	if (g_keys->keyDown[VK_ESCAPE])								
	{
		TerminateApplication (g_window);						
	}

	if (g_keys->keyDown[' '] && game)							
	{
		for (int loop=0; loop<30; loop++)							
			InitObject(loop);										

		game=FALSE;												
		score=0;												
		level=1;												
		kills=0;												
		miss=0;													
	}

	if (g_keys->keyDown[VK_F1])									
	{
		ToggleFullscreen (g_window);							
	}

	roll-=milliseconds*0.00005f;								

	for (int loop=0; loop<level; loop++)						
	{
		if (object[loop].rot==1)								
			object[loop].spin-=0.2f*(float(loop+milliseconds));	

		if (object[loop].rot==2)								
			object[loop].spin+=0.2f*(float(loop+milliseconds));	
		
		if (object[loop].dir==1)								
			object[loop].x+=0.012f*float(milliseconds);			

		if (object[loop].dir==0)								
			object[loop].x-=0.012f*float(milliseconds);			

		if (object[loop].dir==2)								
			object[loop].y+=0.012f*float(milliseconds);			

		if (object[loop].dir==3)								
			object[loop].y-=0.0025f*float(milliseconds);		

		
		if ((object[loop].x<(object[loop].distance-15.0f)/2.0f) && (object[loop].dir==0) && !object[loop].hit)
		{
			miss+=1;											
			object[loop].hit=TRUE;								
		}

		
		if ((object[loop].x>-(object[loop].distance-15.0f)/2.0f) && (object[loop].dir==1) && !object[loop].hit)
		{
			miss+=1;											
			object[loop].hit=TRUE;								
		}

		
		if ((object[loop].y<-2.0f) && (object[loop].dir==3) && !object[loop].hit)
		{
			miss+=1;											
			object[loop].hit=TRUE;								
		}

		if ((object[loop].y>4.5f) && (object[loop].dir==2))		
			object[loop].dir=3;									
	}
}

void Object(float width,float height,GLuint texid)				
{
	glBindTexture(GL_TEXTURE_2D, textures[texid].texID);		
	glBegin(GL_QUADS);											
		glTexCoord2f(0.0f,0.0f); glVertex3f(-width,-height,0.0f);	
		glTexCoord2f(1.0f,0.0f); glVertex3f( width,-height,0.0f);	
		glTexCoord2f(1.0f,1.0f); glVertex3f( width, height,0.0f);	
		glTexCoord2f(0.0f,1.0f); glVertex3f(-width, height,0.0f);	
	glEnd();													
}

void Explosion(int num)											
{
	float ex = (float)((object[num].frame/4)%4)/4.0f;			
	float ey = (float)((object[num].frame/4)/4)/4.0f;			

	glBindTexture(GL_TEXTURE_2D, textures[5].texID);			
	glBegin(GL_QUADS);											
		glTexCoord2f(ex      ,1.0f-(ey      )); glVertex3f(-1.0f,-1.0f,0.0f);	
		glTexCoord2f(ex+0.25f,1.0f-(ey      )); glVertex3f( 1.0f,-1.0f,0.0f);	
		glTexCoord2f(ex+0.25f,1.0f-(ey+0.25f)); glVertex3f( 1.0f, 1.0f,0.0f);	
		glTexCoord2f(ex      ,1.0f-(ey+0.25f)); glVertex3f(-1.0f, 1.0f,0.0f);	
	glEnd();													

	object[num].frame+=1;										
	if (object[num].frame>63)									
	{
		InitObject(num);										
	}
}

void DrawTargets(void)											
{
	glLoadIdentity();											
	glTranslatef(0.0f,0.0f,-10.0f);								
	for (int loop=0; loop<level; loop++)						
	{
		glLoadName(loop);										
		glPushMatrix();											
		glTranslatef(object[loop].x,object[loop].y,object[loop].distance);		
		if (object[loop].hit)									
		{
			Explosion(loop);									
		}
		else													
		{
			glRotatef(object[loop].spin,0.0f,0.0f,1.0f);		
			Object(size[object[loop].texid].w,size[object[loop].texid].h,object[loop].texid);	
		}
		glPopMatrix();											
	}
}

void Draw(void)													
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
	glLoadIdentity();											

	glPushMatrix();												
	glBindTexture(GL_TEXTURE_2D, textures[7].texID);			
	glBegin(GL_QUADS);											
		glTexCoord2f(1.0f,roll/1.5f+1.0f); glVertex3f( 28.0f,+7.0f,-50.0f);	
		glTexCoord2f(0.0f,roll/1.5f+1.0f); glVertex3f(-28.0f,+7.0f,-50.0f);	
		glTexCoord2f(0.0f,roll/1.5f+0.0f); glVertex3f(-28.0f,-3.0f,-50.0f);	
		glTexCoord2f(1.0f,roll/1.5f+0.0f); glVertex3f( 28.0f,-3.0f,-50.0f);	

		glTexCoord2f(1.5f,roll+1.0f); glVertex3f( 28.0f,+7.0f,-50.0f);		
		glTexCoord2f(0.5f,roll+1.0f); glVertex3f(-28.0f,+7.0f,-50.0f);		
		glTexCoord2f(0.5f,roll+0.0f); glVertex3f(-28.0f,-3.0f,-50.0f);		
		glTexCoord2f(1.5f,roll+0.0f); glVertex3f( 28.0f,-3.0f,-50.0f);		

		glTexCoord2f(1.0f,roll/1.5f+1.0f); glVertex3f( 28.0f,+7.0f,0.0f);	
		glTexCoord2f(0.0f,roll/1.5f+1.0f); glVertex3f(-28.0f,+7.0f,0.0f);	
		glTexCoord2f(0.0f,roll/1.5f+0.0f); glVertex3f(-28.0f,+7.0f,-50.0f);	
		glTexCoord2f(1.0f,roll/1.5f+0.0f); glVertex3f( 28.0f,+7.0f,-50.0f);	

		glTexCoord2f(1.5f,roll+1.0f); glVertex3f( 28.0f,+7.0f,0.0f);		
		glTexCoord2f(0.5f,roll+1.0f); glVertex3f(-28.0f,+7.0f,0.0f);		
		glTexCoord2f(0.5f,roll+0.0f); glVertex3f(-28.0f,+7.0f,-50.0f);		
		glTexCoord2f(1.5f,roll+0.0f); glVertex3f( 28.0f,+7.0f,-50.0f);		
	glEnd();													

	glBindTexture(GL_TEXTURE_2D, textures[6].texID);			
	glBegin(GL_QUADS);											
		glTexCoord2f(7.0f,4.0f-roll); glVertex3f( 27.0f,-3.0f,-50.0f);	
		glTexCoord2f(0.0f,4.0f-roll); glVertex3f(-27.0f,-3.0f,-50.0f);	
		glTexCoord2f(0.0f,0.0f-roll); glVertex3f(-27.0f,-3.0f,0.0f);	
		glTexCoord2f(7.0f,0.0f-roll); glVertex3f( 27.0f,-3.0f,0.0f);	
	glEnd();													

	DrawTargets();												
	glPopMatrix();												

	
	RECT window;												
	GetClientRect (g_window->hWnd,&window);						
	glMatrixMode(GL_PROJECTION);								
	glPushMatrix();												
	glLoadIdentity();											
	glOrtho(0,window.right,0,window.bottom,-1,1);				
	glMatrixMode(GL_MODELVIEW);									
	glTranslated(mouse_x,window.bottom-mouse_y,0.0f);			
	Object(16,16,8);											

	
	glPrint(240,450,"LR32");						
	glPrint(10,10,"Level: %i",level);							
	glPrint(250,10,"Score: %i",score);							

	if (miss>9)													
	{
		miss=9;													
		game=TRUE;												
	}

	if (game)													
		glPrint(490,10,"GAME OVER");							
	else
		glPrint(490,10,"Morale: %i/10",10-miss);				

	glMatrixMode(GL_PROJECTION);								
	glPopMatrix();												
	glMatrixMode(GL_MODELVIEW);									

	glFlush();													
}
