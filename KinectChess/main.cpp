#include <stdlib.h>
#include "SDL\SDL.h"
#include "GLEngine.h"
#include "Control.h"
#include "camera.h"

// Headers for OpenNI
/*#include <XnOpenNI.h>
#include <XnCppWrapper.h>
#include <XnHash.h>
#include <XnLog.h>

// Header for NITE
#include "XnVNite.h"*/
// local header
//#include "PointDrawer.h"

#define GLUT_KEY_ESCAPE		27

#define CHESS_BOARD			"../KinectChess/Content/Models/chess-finish.obj"

//multi-platform
GLsizei windowWidth = 800;
GLsizei windowHeight = 600;


Control *controlled = NULL; //holds pointer to control currently being modified
bool navigating = false; //for mouse control
MouseState state;
Uint8 *keys = NULL;

int     MouseX = 0,
        MouseY = 0,
        MouseButton = 0,
        RelX = 0,
        RelY = 0;

Camera cam;

GLvoid establishProjectionMatrix(GLsizei width, GLsizei height)
{
	//x, y, width, height
	glViewport(0, 0, width, height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	//cast to float because vars are int's
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 2000.0f); //adjust for clipping near / far
}

GLvoid drawScene(bool selection = false)
{
	if ( selection == false )
	{
		//need to clear the color & depth in 3D view
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//cam.Perspective(0, 0, windowWidth, windowHeight);
		establishProjectionMatrix(windowWidth, windowHeight);
	}

    cam.SetCamera();

	if ( iGLEngine->supportProgrammablePipeline() )
		iGLEngine->drawModelUsingProgrammablePipeline();

	if ( selection == true )
		return;

	glDisable(GL_LIGHTING);
	cam.Orthographic(0, 0, windowWidth, windowHeight);
    cam.ClearCamera();

	glFlush();
	//this line is modified in other window interfaces; takes back buffer into view buffer
	SDL_GL_SwapBuffers();
}

void handleSelections(void)
{
	static bool buttonDown = false;

	if ( state.LeftButtonDown )
		buttonDown = true;
	else if ( buttonDown )
	{
		buttonDown = false;

		if ( !iGLEngine->getIsSquareSelected() && iGLEngine->getIsItemSelected() )
		{
			GLubyte pixel[1];
			GLint viewport[4];

			glGetIntegerv(GL_VIEWPORT, viewport);
			glReadPixels(state.x,viewport[3] - state.y,1,1, GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,(void *)pixel);

			ModelOBJ::GroupObject *selectedItem = iGLEngine->getObject( (int)pixel[0] );
			iGLEngine->setSelectedSquare( selectedItem );
			iGLEngine->squareSelected( true );

			ModelOBJ::GroupObject *piece = iGLEngine->getSelectedItem();
			Vector3 moveTo = selectedItem->center.operator-( piece->square->center );
			piece->square = selectedItem;
			piece->position.setTranslate( moveTo.x, moveTo.y, moveTo.z );
			piece->position.moved = true;
			iGLEngine->setMovePoint(moveTo);
		}

		if ( !iGLEngine->getIsItemSelected() )
		{
			GLubyte pixel[1];
			GLint viewport[4];

			glGetIntegerv(GL_VIEWPORT, viewport);
			glReadPixels(state.x,viewport[3] - state.y,1,1, GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,(void *)pixel);

			if ( (int)pixel[0] != 0 )
			{
				ModelOBJ::GroupObject *selectedItem = iGLEngine->getObject( (int)pixel[0] );
				int pos = selectedItem->objectName.rfind("Object",0);
				if ( selectedItem->objectName != "Board" && pos == -1 )
				{
					iGLEngine->setSelectedItem( selectedItem );
					iGLEngine->itemSelected( true );
				}
			}
		}
	}
}

GLvoid displayFPS()
{
	//execute first call, then we can refer back to it
	static long lastTime = SDL_GetTicks();
	//how many times has this function been called?
	static long loops = 0;
	//store fps of last run through
	static GLfloat fps = 0.0f;

	int newTime = SDL_GetTicks();

	//have we elapsed 100 miliseconds?
	if (newTime - lastTime > 100)
	{
		float newFPS = (float)loops / float(newTime - lastTime) * 1000.0f;
		//avg. FPS
		fps = (fps + newFPS) / 2.0f;

		lastTime = newTime;
		loops = 0;
	}
	loops++;
	iGLEngine->drawText(5, 5, "FPS - %.2f", fps);
}

void MouseHandler()
{
    if (MouseButton == 1)
        cam.Orbit(RelX, RelY);
    else if (MouseButton == 4)
        cam.Pan(RelX,RelY);
    else if (MouseButton == 5)
        cam.Zoom( RelY );
    else if (MouseButton == 8)
        cam.Zoom( 0.8f );
    else if (MouseButton == 16)
        cam.Zoom( 1.2f );
}

void MouseClick(int button, int state, int x, int y)
{
    unsigned int b = (1 << button-1);
    if (state)
        MouseButton = MouseButton | b;
    else
        MouseButton = MouseButton & (~b);
    MouseX = x;
    MouseY = y;
}

void MouseMove(int x, int y)
{
    RelX = x - MouseX;
    RelY = y - MouseY;
    MouseX = x;
    MouseY = y;
}

void Resize(int w, int h)
{
    windowWidth = w;
    windowHeight = h;
}


int main (int argc, char* argv[])
{

	if (SDL_Init(SDL_INIT_VIDEO) < 0 )
	{
		//spit out error
		fprintf(stderr, "Unable to initialize to SDL %s", SDL_GetError());
		exit(1);
	}

	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 1 ); //NWB

	//SDL_SetVideoMode
	if ( SDL_SetVideoMode(windowWidth, windowHeight, 0, SDL_OPENGL | SDL_RESIZABLE) == NULL)
	{
		//spit out error
		fprintf(stderr, "Unable to create opengl scene %s", SDL_GetError());
		exit(2);
	}

	iGLEngine->Initialize(windowWidth, windowHeight);

    iGLEngine->loadModel( CHESS_BOARD);
	//ResetCamera();

	int done = 0;
	while ( !done )
	{
		SDL_GetMouseState(&state.x, &state.y);
		state.LeftButtonDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1);
		state.MiddleButtonDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(2);
		state.RightButtonDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(3);
		//state.WheelUp = SDL_PollEvent(NULL) & SDL_BUTTON(SDL_BUTTON_WHEELUP);
		//state.WheelDown = SDL_PollEvent(NULL) & SDL_BUTTON(SDL_BUTTON_WHEELDOWN);

		drawScene();

		handleSelections();

		SDL_Event event;
		while (SDL_PollEvent(&event) )
		{
			if ( event.type == SDL_VIDEORESIZE)
            {
                Resize(event.resize.w, event.resize.h);
            }
            else if ( event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP )
            {
                MouseClick(event.button.button, event.button.state, event.button.x, event.button.y);
                MouseHandler();
            }
            else if ( event.type == SDL_MOUSEMOTION )
            {
                MouseMove(event.motion.x, event.motion.y);
                MouseHandler();
            }
			else if (event.type == SDL_QUIT )
			{
				done = 1;
			}

			keys = SDL_GetKeyState(NULL);
		}
	}
	while (Control::controls.size() > 1)
		delete (*Control::controls.begin());

	GLEngine::Uninitialize();

	SDL_Quit();

	return 1;
}


