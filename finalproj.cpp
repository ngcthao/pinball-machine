#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#endif


#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// title of these windows:

const char *WINDOWTITLE = "OpenGL / Final Project -- Ngoc-Thao Ly";
const char *GLUITITLE   = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE  = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 600;

// size of the 3d box to be drawn:

const float BOXSIZE = 15.f;
const float NUMSEGS = 20.f;
const float RADIUS = 5.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT   = 4;
const int MIDDLE = 2;
const int RIGHT  = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE     = GL_LINEAR;
const GLfloat FOGDENSITY  = 0.30f;
const GLfloat FOGSTART    = 1.5f;
const GLfloat FOGEND      = 4.f;

// for lighting:

const float	WHITE[ ] = { 1.,1.,1.,1. };

// for animation:

const int MS_PER_CYCLE = 11000;		// 10000 milliseconds = 10 seconds


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
int		NowColor;				// index into Colors[ ]
int		NowProjection;		// ORTHO or PERSP
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
float	Time;					// used for animation, this has a value between 0. and 1.
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees


// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void			Axes( float );
void			HsvRgb( float[3], float [3] );
void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);
float			Unit(float [3]);


// utility to create an array from 3 separate values:

float *
Array3( float a, float b, float c )
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float *
MulArray3( float factor, float array0[ ] )
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}


float *
MulArray3(float factor, float a, float b, float c )
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}

// these are here for when you need them -- just uncomment the ones you need:

#include "setmaterial.cpp"
#include "setlight.cpp"
#include "osusphere.cpp"
//#include "osucone.cpp"
//#include "osutorus.cpp"
#include "bmptotexture.cpp"
#include "loadobjfile.cpp"
#include "keytime.cpp"
//#include "glslprogram.cpp"

const int ScaleFactor = 60;

GLuint			GridDL;
GLuint			SphereDL;
GLuint			PlungerDL;
GLuint			LeverDL;
GLuint			TopPlateDL;
GLuint			BottomPlateDL;
GLuint			CircleStaticDL;
GLuint			TriangleStaticDL;
GLuint			CrossDL;
GLuint			StarDL;

GLuint			SpaceTex;

Keytimes		BallX;
Keytimes		BallZ;
Keytimes		StarRot;
Keytimes		CrossRot;
Keytimes		LeverL;
Keytimes		LeverR;
Keytimes		PlungerZ;

Keytimes		PosZ, LookY;

Keytimes		StarR, StarG, StarB;
Keytimes		CrossR, CrossG, CrossB;
Keytimes		TriangleR, TriangleG, TriangleB;

int				LightSwitch = 0.0;

#define XSIDE	100				// length of the x side of the grid
#define X0      ( -XSIDE/2.0f )		// where one side starts
#define NX		1000			// how many points in x
#define DX		( XSIDE/(float)NX )	// change in x between the points

#define YGRID	0.f

#define ZSIDE	100				// length of the z side of the grid
#define Z0      (-ZSIDE/2.)		// where one side starts
#define NZ		1000			// how many points in z
#define DZ		( ZSIDE/(float)NZ )	// change in z between the points

// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// create the display lists that **will not change**:

	InitLists( );

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	// for example, if you wanted to spin an object in Display( ), you might call: glRotatef( 360.f*Time,   0., 1., 0. );

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );

	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif


	// specify shading to be flat:

	glShadeModel( GL_FLAT );

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( NowProjection == ORTHO )
		glOrtho( -2.f, 2.f,     -2.f, 2.f,     0.1f, 1000.f );
	else
		gluPerspective( 70.f, 1.f,	0.1f, 1000.f );

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0.f, 1.f, 0.f );
	glRotatef( (GLfloat)Xrot, 1.f, 0.f, 0.f );

	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// possibly draw the axes:

	//if( AxesOn != 0 )
	//{
	//	glColor3fv( &Colors[NowColor][0] );
	//	glCallList( AxesList );
	//}

	// since we are using glScalef( ), be sure the normals get unitized:
	glEnable( GL_NORMALIZE );
	glShadeModel(GL_SMOOTH);

	// enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);

	// turn # msec into the cycle ( 0 - MSEC-1 ):
	int msec = glutGet(GLUT_ELAPSED_TIME) % MS_PER_CYCLE;
	// turn that into a time in seconds:
	float nowTime = (float)msec / 1000.;

	// set the eye position, look-at position, and up-vector:
	if (NowProjection == ORTHO) { gluLookAt(0.f, 14.f, 0.f, 0.f, 0.0f, 0.f, 0.f, 0.f, -1.f); }
	else { gluLookAt(0.f, 14.f, PosZ.GetValue(nowTime), 0.f, LookY.GetValue(nowTime), 0.f, 0.f, 0.f, -1.f); }

	glPushMatrix();
		glTranslatef(BallX.GetValue(nowTime), 2.3f, BallZ.GetValue(nowTime));
		SetSpotLight(GL_LIGHT1, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	glPopMatrix();

	// enable textures
	glEnable(GL_TEXTURE_2D);

	if (LightSwitch == 0.0) {
		glDisable(GL_LIGHT2);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		SetPointLight(GL_LIGHT0, -0.6, 2, 6.8, 0.2, 0.2, 0.5);
	}
	else if (LightSwitch == 1.0) {
		glEnable(GL_LIGHT2);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		SetSpotLight(GL_LIGHT0, -1.6f, 5.0f, 6.7f, -0.5f, -0.5f, -1.0f, 0.4f, 0.0f, 0.2f);
		SetSpotLight(GL_LIGHT2, 0.4f, 5.0f, 6.7f, 0.5f, -0.5f, -1.0f, 0.3f, 0.0f, 0.4f);
	}
	else if (LightSwitch == 2.0) {
		glEnable(GL_LIGHT2);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		SetPointLight(GL_LIGHT0, 0, 20, 0, 0.01, 0.01, 0.01);
		SetSpotLight(GL_LIGHT2, 0.0f, 15.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.1f, 0.1f, 0.2f);
	}
	else {
		glDisable(GL_LIGHT2);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		SetPointLight(GL_LIGHT0, 0, 5, 0, 0.9, 0.9, 1.0); 
	}

	// bottom plate
	glPushMatrix();
	glCallList(BottomPlateDL);
	glPopMatrix();

	// disable textures
	glDisable(GL_TEXTURE_2D);

	// pinball
	glPushMatrix();
	glTranslatef(BallX.GetValue(nowTime), 1.8, BallZ.GetValue(nowTime));
	glCallList(SphereDL);
	glPopMatrix();

	// static triangle
	glPushMatrix();
	glTranslatef(2.6, 1.8, 2.9);
	SetMaterial(TriangleR.GetValue(nowTime), TriangleG.GetValue(nowTime), TriangleB.GetValue(nowTime), 128.f);
	glCallList(TriangleStaticDL);
	glPopMatrix();

	// static circle 2
	glPushMatrix();
	glTranslatef(2.4, 1.8, -0.5);
	SetMaterial(0.8f, 0.7f, 0.3f, 128.f);
	glCallList(CircleStaticDL);
	glPopMatrix();

	// static circle 1
	glPushMatrix();
	glTranslatef(-2.4, 1.8, -0.5);
	SetMaterial(0.8f, 0.7f, 0.3f, 128.f);
	glCallList(CircleStaticDL);
	glPopMatrix();

	// cross
	glPushMatrix();
	glTranslatef(0, 1.8, -3);
	glRotatef(CrossRot.GetValue(nowTime), 0, 1, 0);
	SetMaterial(CrossR.GetValue(nowTime), CrossG.GetValue(nowTime), CrossB.GetValue(nowTime), 128.f);
	glCallList(CrossDL);
	glPopMatrix();

	// star
	glPushMatrix();
	glTranslatef(-3.5, 1.8, 2.7);
	glRotatef(StarRot.GetValue(nowTime), 0, 1, 0);
	SetMaterial(StarR.GetValue(nowTime), StarG.GetValue(nowTime), StarB.GetValue(nowTime), 128.f);
	glCallList(StarDL);
	glPopMatrix();

	// left lever
	glPushMatrix();
	glTranslatef(-2, 1.8, 5.26);
	glRotatef(LeverL.GetValue(nowTime), 0, 1, 0);
	glRotatef(-130, 0, 1, 0);
	glCallList(LeverDL);
	glPopMatrix();

	// right lever
	glPushMatrix();
	glTranslatef(0.97, 1.8, 5.26);
	glRotatef(LeverR.GetValue(nowTime), 0, 1, 0);
	glRotatef(130, 0, 1, 0);
	glCallList(LeverDL);
	glPopMatrix();

	// plunger
	glPushMatrix();
	glTranslatef(4.95, 1.8, PlungerZ.GetValue(nowTime));
	glCallList(PlungerDL);
	glPopMatrix();

	// top plate
	glPushMatrix();
	SetMaterial(0.3f, 0.5f, 0.6f, 0.f);
	glCallList(TopPlateDL);
	glPopMatrix();

	// left grid
	glPushMatrix();
	glTranslatef(-XSIDE / 2 + 5, 0, 0);
	glRotatef(90, 0, 0, 1);
	glCallList(GridDL);
	glPopMatrix();

	// right grid
	glPushMatrix();
	glTranslatef(XSIDE / 2 - 5, 0, 0);
	glRotatef(90, 0, 0, 1);
	glCallList(GridDL);
	glPopMatrix();

	// front grid
	glPushMatrix();
	glTranslatef(0, 0, -ZSIDE/2);
	glRotatef(90, 1, 0, 0);
	glCallList(GridDL);
	glPopMatrix();

	// back grid
	glPushMatrix();
	glTranslatef(0, 0, ZSIDE / 2);
	glRotatef(90, 1, 0, 0);
	glCallList(GridDL);
	glPopMatrix();

	// bottom grid
	glPushMatrix();
	glTranslatef(0, -1, 0);
	glCallList(GridDL);
	glPopMatrix();

	glDisable(GL_LIGHTING);

#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.f,   0.f, 1.f, 0.f );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif


	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0.f, 1.f, 1.f );
	//DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0.f, 100.f,     0.f, 100.f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1.f, 1.f, 1.f );
	//DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	NowColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	NowProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(float) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Axis Colors",   colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc( Animate );

	float starRed = 0.2;
	float starGreen = 0.35;
	float starBlue = 0.45;
	float crossRed = 0.45;
	float crossGreen = 0.2;
	float crossBlue = 0.4;

	StarR.Init();
		StarR.AddTimeValue(6.55, 0.8);
		StarR.AddTimeValue(6.6, starRed);
		StarR.AddTimeValue(8.3, starRed);
		StarR.AddTimeValue(8.35, 0.8);

	StarG.Init();
		StarG.AddTimeValue(6.55, 0.7);
		StarG.AddTimeValue(6.6, starGreen);
		StarG.AddTimeValue(8.3, starGreen);
		StarG.AddTimeValue(8.35, 0.7);

	StarB.Init();
		StarB.AddTimeValue(6.55, 0.3);
		StarB.AddTimeValue(6.6, starBlue);
		StarB.AddTimeValue(8.3, starBlue);
		StarB.AddTimeValue(8.35, 0.3);

	CrossR.Init();
		CrossR.AddTimeValue(7.15, 0.8);
		CrossR.AddTimeValue(7.2, crossRed);
		CrossR.AddTimeValue(7.4, 0.8);

	CrossG.Init();
		CrossG.AddTimeValue(7.15, 0.7);
		CrossG.AddTimeValue(7.2, crossGreen);
		CrossG.AddTimeValue(7.4, 0.7);

	CrossB.Init();
		CrossB.AddTimeValue(7.15, 0.3);
		CrossB.AddTimeValue(7.2, crossBlue);
		CrossB.AddTimeValue(7.5, 0.3);

	TriangleR.Init();
		TriangleR.AddTimeValue(9.35, 0.8);
		TriangleR.AddTimeValue(9.4, starRed);
		TriangleR.AddTimeValue(9.45, 0.8);

	TriangleG.Init();
		TriangleG.AddTimeValue(9.35, 0.7);
		TriangleG.AddTimeValue(9.4, starGreen);
		TriangleG.AddTimeValue(9.45, 0.7);

	TriangleB.Init();
		TriangleB.AddTimeValue(9.3, 0.3);
		TriangleB.AddTimeValue(9.4, starBlue);
		TriangleB.AddTimeValue(9.5, 0.3);
	
	PosZ.Init();
		PosZ.AddTimeValue(0.0, 35);
		PosZ.AddTimeValue(3.0, 5);
	
	LookY.Init();
		LookY.AddTimeValue(3.0, 10);
		LookY.AddTimeValue(4.0, 0);

	PlungerZ.Init();
		PlungerZ.AddTimeValue(4.0, 6);
		PlungerZ.AddTimeValue(6.0, 7.5);
		PlungerZ.AddTimeValue(6.05, 6);

	BallX.Init();
		// moving pinball
		BallX.AddTimeValue(6.1, 4.95);
		BallX.AddTimeValue(6.2, 3.2);
		BallX.AddTimeValue(6.3, 0);
		BallX.AddTimeValue(6.4, -3.2);
		BallX.AddTimeValue(6.5, -4.95);
		// exit the road
		BallX.AddTimeValue(6.6, -4);	 // hit the star
		BallX.AddTimeValue(6.64, -3.08); // hit the star
		BallX.AddTimeValue(6.67, -2.24); // hit the star
		// hit the right lever
		BallX.AddTimeValue(6.8, 0.05);	//exaggerate impact
		BallX.AddTimeValue(6.95, 0.05);	// move down
		BallX.AddTimeValue(7.0, 0.1);	//exaggerate impact
		BallX.AddTimeValue(7.3, 1.46);	// hit the cross
		// enter the chaos
		BallX.AddTimeValue(7.7, -3.88);	// hit the cross
		BallX.AddTimeValue(7.8, -3.24);
		BallX.AddTimeValue(7.9, -3.95);
		BallX.AddTimeValue(8.0, -3.55);
		BallX.AddTimeValue(8.1, -3.55);
		BallX.AddTimeValue(8.2, -3.38);
		// exit the chaos
		BallX.AddTimeValue(8.3, -3.1);
		BallX.AddTimeValue(8.4, -3.14);	// hit the star
		BallX.AddTimeValue(8.6, -2.19); // hit the star
		BallX.AddTimeValue(8.7, -1.7);
		// hit left lever
		BallX.AddTimeValue(8.9, -1.42);	
		BallX.AddTimeValue(9, -1.34);	//exaggerate impact
		BallX.AddTimeValue(9.05, -0.8);
		// hit right circle
		BallX.AddTimeValue(9.3, 1.67);
		// hit the triangle
		BallX.AddTimeValue(9.4, 2.19);
		// hit the left circle
		BallX.AddTimeValue(9.6, -1.64);
		// hit the right lever
		BallX.AddTimeValue(10.0, 0.43);
		BallX.AddTimeValue(10.05, 0.1);
		BallX.AddTimeValue(10.1, -0.3);
		BallX.AddTimeValue(10.3, -1);

	BallZ.Init();
		// plunger movement
		BallZ.AddTimeValue(4.0, 4.3);
		BallZ.AddTimeValue(6.0, 5.8);
		BallZ.AddTimeValue(6.05, 4.3);
		// moving pinball
		BallZ.AddTimeValue(6.1, -1.5);
		BallZ.AddTimeValue(6.2, -5.3);
		BallZ.AddTimeValue(6.3, -6.5);
		BallZ.AddTimeValue(6.4, -5.3);
		BallZ.AddTimeValue(6.5, -1.5);
		// exit the road
		BallZ.AddTimeValue(6.6, 1.5);	// hit the star
		BallZ.AddTimeValue(6.64, 1.6);	// hit the star
		BallZ.AddTimeValue(6.67, 2.6);	// hit the star
		// hit the right lever
		BallZ.AddTimeValue(6.8, 5.13);	//exaggerate impact
		BallZ.AddTimeValue(6.95, 5.4);	// move down
		BallZ.AddTimeValue(7.0, 5.25);	//exaggerate impact
		BallZ.AddTimeValue(7.3, -5);	// hit the cross
		// enter the chaos
		BallZ.AddTimeValue(7.7, -2.29);	// hit the cross
		BallZ.AddTimeValue(7.8, -1.26);
		BallZ.AddTimeValue(7.9, -1.5);
		BallZ.AddTimeValue(8.0, -0.7);
		BallZ.AddTimeValue(8.1, 0.1);
		BallZ.AddTimeValue(8.2, 0.05);
		// exit the chaos
		BallZ.AddTimeValue(8.3, 1.04);
		BallZ.AddTimeValue(8.4, 1.33);	// hit the star
		BallZ.AddTimeValue(8.6, 2.36);	// hit the star
		BallZ.AddTimeValue(8.7, 3.25);
		// hit left lever
		BallZ.AddTimeValue(8.9, 4.94);
		BallZ.AddTimeValue(9, 5.56);	//exaggerate impact
		BallZ.AddTimeValue(9.05, 5.0);
		// hit right circle
		BallZ.AddTimeValue(9.3, 0.46);
		// hit the triangle
		BallZ.AddTimeValue(9.4, 2.83);
		// hit the left circle
		BallZ.AddTimeValue(9.6, 0.3);
		// hit right lever
		BallZ.AddTimeValue(10.0, 4.86);
		BallZ.AddTimeValue(10.05, 5.25);
		BallZ.AddTimeValue(10.1, 5.74);
		BallZ.AddTimeValue(10.3, 7.4);


	StarRot.Init();
		StarRot.AddTimeValue(6.6, 0);
		StarRot.AddTimeValue(6.8, -360 * 1.8);
		StarRot.AddTimeValue(8, -360 * 2.73);

		StarRot.AddTimeValue(8.4, -360 * 2.73);
		StarRot.AddTimeValue(8.75, -360 * 3);
		StarRot.AddTimeValue(11, -360 * 3.25);

	CrossRot.Init();
		CrossRot.AddTimeValue(7.2, 0);
		CrossRot.AddTimeValue(7.23, 50);
		CrossRot.AddTimeValue(7.5, 100);
		CrossRot.AddTimeValue(8.5, 160);
		CrossRot.AddTimeValue(8.8, 180);

	LeverL.Init();
		LeverL.AddTimeValue(8.9, 0);
		LeverL.AddTimeValue(8.95, -35);
		LeverL.AddTimeValue(9.0, -35);
		LeverL.AddTimeValue(9.05, 0);

	LeverR.Init();
		LeverR.AddTimeValue(6.8, 0);
		LeverR.AddTimeValue(6.85, 30);
		LeverR.AddTimeValue(6.95, 30);
		LeverR.AddTimeValue(7.0, 0);

	// Space Texture
	int width, height;
	char* file = (char*)"space.bmp";
	unsigned char* texture = BmpToTexture(file, &width, &height);
	if (texture == NULL)
		fprintf(stderr, "Cannot open texture '%s'\n", file);
	else
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file, width, height);

	glGenTextures(1, &SpaceTex);
	glBindTexture(GL_TEXTURE_2D, SpaceTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:

}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	glutSetWindow( MainWindow );

	// Create the top plate:
	TopPlateDL = glGenLists(1);
	glNewList(TopPlateDL, GL_COMPILE);
	glRotatef(-90, 1, 0, 0);
	glScalef(ScaleFactor, ScaleFactor, ScaleFactor);
	SetMaterial(0.15f, 0.15f, 0.2f, 10.f);
	LoadObjFile((char*)"Top.obj");
	glEndList();

	// Create the bottom plate:
	float dx = 0.1016;
	float dy = 0.1524;
	float dz = 0.01905;
	BottomPlateDL = glGenLists(1);
	glNewList(BottomPlateDL, GL_COMPILE);
	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, SpaceTex);
		glRotatef(-90, 1, 0, 0);
		glScalef(ScaleFactor, ScaleFactor, ScaleFactor);
		//LoadObjFile((char*)"Bottom.obj");
		glBegin(GL_QUADS);
			
			glTexCoord2f(0.0, 0.0);
			glVertex3f(-dx, -dy, dz);
			glTexCoord2f(1.0, 0.0);
			glVertex3f(dx, -dy, dz);
			glTexCoord2f(1.0, 1.0);
			glVertex3f(dx, dy, dz);
			glTexCoord2f(0.0, 1.0);
			glVertex3f(-dx, dy, dz);

			glVertex3f(-dx, -dy, -dz);
			glVertex3f(dx, -dy, -dz);
			glVertex3f(dx, dy, -dz);
			glVertex3f(-dx, dy, -dz);

			glVertex3f(-dx, -dy, dz);
			glVertex3f(dx, -dy, dz);
			glVertex3f(dx, -dy, -dz);
			glVertex3f(-dx, -dy, -dz);

			glVertex3f(dx, -dy, dz);
			glVertex3f(dx, dy, dz);
			glVertex3f(dx, dy, -dz);
			glVertex3f(dx, -dy, -dz);

			glVertex3f(-dx, -dy, dz);
			glVertex3f(-dx, dy, dz);
			glVertex3f(-dx, dy, -dz);
			glVertex3f(-dx, -dy, -dz);

			glVertex3f(-dx, dy, dz);
			glVertex3f(dx, dy, dz);
			glVertex3f(dx, dy, -dz);
			glVertex3f(-dx, dy, -dz);

		glEnd();
	glPopMatrix();
	glEndList();

	// create the pinball:
	SphereDL = glGenLists(1);
	glNewList(SphereDL, GL_COMPILE);
	SetMaterial(1.f, 1.f, 1.f, 128.f);
	OsuSphere(0.35f, 32, 32);
	glEndList();

	// Create the plunger:
	PlungerDL = glGenLists(1);
	glNewList(PlungerDL, GL_COMPILE);
	glRotatef(-90, 1, 0, 0);
	glScalef(ScaleFactor, ScaleFactor, ScaleFactor);
	SetMaterial(0.8f, 0.7f, 0.3f, 128.f);
	LoadObjFile((char*)"Starter.obj");
	glEndList();

	// Create the lever:
	LeverDL = glGenLists(1);
	glNewList(LeverDL, GL_COMPILE);
	glScalef(ScaleFactor, ScaleFactor, ScaleFactor);
	SetMaterial(0.8f, 0.7f, 0.3f, 128.f);
	LoadObjFile((char*)"Lever.obj");
	glEndList();

	// Create the cross:
	CrossDL = glGenLists(1);
	glNewList(CrossDL, GL_COMPILE);
	glScalef(ScaleFactor, ScaleFactor, ScaleFactor);
	LoadObjFile((char*)"Sparkle.obj");
	glEndList();

	// Create the static circle:
	CircleStaticDL = glGenLists(1);
	glNewList(CircleStaticDL, GL_COMPILE);
	glScalef(ScaleFactor, ScaleFactor, ScaleFactor);
	LoadObjFile((char*)"Circle.obj");
	glEndList();

	// Create the static star:
	StarDL = glGenLists(1);
	glNewList(StarDL, GL_COMPILE);
	glScalef(ScaleFactor, ScaleFactor, ScaleFactor);
	LoadObjFile((char*)"Star.obj");
	glEndList();

	// Create the static triangle:
	TriangleStaticDL = glGenLists(1);
	glNewList(TriangleStaticDL, GL_COMPILE);
	glRotatef(-30, 0, 1, 0);
	glScalef(ScaleFactor, ScaleFactor, ScaleFactor);
	LoadObjFile((char*)"Triangle.obj");
	glEndList();

	// create the axes:
	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );

	// create the grid:
	GridDL = glGenLists(1);
	glNewList(GridDL, GL_COMPILE);
	SetMaterial(0.5f, 0.5f, 0.6f, 30.f);
	glNormal3f(0., 1., 0.);
	for (int i = 0; i < NZ; i++)
	{
		glBegin(GL_QUAD_STRIP);
		for (int j = 0; j < NX; j++)
		{
			glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 0));
			glVertex3f(X0 + DX * (float)j, YGRID, Z0 + DZ * (float)(i + 1));
		}
		glEnd();
	}
	glEndList();
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			NowProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			NowProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		case 'l':
		case 'L':
			if (LightSwitch == 0.0) { LightSwitch = 1.0; }
			else if (LightSwitch == 1.0) { LightSwitch = 2.0; }
			else if (LightSwitch == 2.0) { LightSwitch = 3.0; }
			else if (LightSwitch == 3.0) { LightSwitch = 4.0; }
			else { LightSwitch = 0.0; }
			break;

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	ShadowsOn = 0;
	NowColor = YELLOW;
	NowProjection = PERSP;
	Xrot = Yrot = 0.;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = { 0.f, 1.f, 0.f, 1.f };

static float xy[ ] = { -.5f, .5f, .5f, -.5f };

static int xorder[ ] = { 1, 2, -3, 4 };

static float yx[ ] = { 0.f, 0.f, -.5f, .5f };

static float yy[ ] = { 0.f, .6f, 1.f, 1.f };

static int yorder[ ] = { 1, 2, 3, -2, 4 };

static float zx[ ] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[ ] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[ ] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}


float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}


float
Unit( float v[3] )
{
	float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}
	return dist;
}
