#include <stdlib.h>
#include "SDL\SDL.h"
#include "GLEngine.h"
#include "Control.h"
#include "camera.h"

// Headers for OpenNI
#include <XnOpenNI.h>
#include <XnCppWrapper.h>
#include <XnHash.h>
#include <XnLog.h>

// Header for NITE
#include "XnVNite.h"
// local header
#include "PointDrawer.h"

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

//----------------------KINECT---------------------
#define CHECK_RC(rc, what)											\
	if (rc != XN_STATUS_OK)											\
	{																\
		printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
		return rc;													\
	}

#define CHECK_ERRORS(rc, errors, what)		\
	if (rc == XN_STATUS_NO_NODE_PRESENT)	\
	{										\
		XnChar strError[1024];				\
		errors.ToString(strError, 1024);	\
		printf("%s\n", strError);			\
		return (rc);						\
	}

// OpenNI objects
xn::Context g_Context;
xn::DepthGenerator g_DepthGenerator;
xn::HandsGenerator g_HandsGenerator;

// NITE objects
XnVSessionManager* g_pSessionManager;
XnVFlowRouter* g_pFlowRouter;

// the drawer
XnVPointDrawer* g_pDrawer;

#define GL_WIN_SIZE_X 720
#define GL_WIN_SIZE_Y 480

// Draw the depth map?
XnBool g_bDrawDepthMap = true;
XnBool g_bPrintFrameID = false;
// Use smoothing?
XnFloat g_fSmoothing = 0.0f;
XnBool g_bPause = false;
XnBool g_bQuit = false;

SessionState g_SessionState = NOT_IN_SESSION;

// Callback for when the focus is in progress
void XN_CALLBACK_TYPE FocusProgress(const XnChar* strFocus, const XnPoint3D& ptPosition, XnFloat fProgress, void* UserCxt)
{
//	printf("Focus progress: %s @(%f,%f,%f): %f\n", strFocus, ptPosition.X, ptPosition.Y, ptPosition.Z, fProgress);
}
// callback for session start
void XN_CALLBACK_TYPE SessionStarting(const XnPoint3D& ptPosition, void* UserCxt)
{
	printf("Session start: (%f,%f,%f)\n", ptPosition.X, ptPosition.Y, ptPosition.Z);
	g_SessionState = IN_SESSION;
}
// Callback for session end
void XN_CALLBACK_TYPE SessionEnding(void* UserCxt)
{
	printf("Session end\n");
	g_SessionState = NOT_IN_SESSION;
}
void XN_CALLBACK_TYPE NoHands(void* UserCxt)
{
	if (g_SessionState != NOT_IN_SESSION)
	{
		printf("Quick refocus\n");
		g_SessionState = QUICK_REFOCUS;
	}
}

//----------------------KINECT---------------------

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

// xml to initialize OpenNI
#define SAMPLE_XML_PATH "Sample-Tracking.xml"

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
	
	 /* Set up the SDL_TTF */
	TTF_Init();
	atexit(TTF_Quit);

	//-----------------KINECT---------------------------

	XnStatus rc = XN_STATUS_OK;
	xn::EnumerationErrors errors;

	// Initialize OpenNI
	rc = g_Context.InitFromXmlFile(SAMPLE_XML_PATH, &errors);
	CHECK_ERRORS(rc, errors, "InitFromXmlFile");
	CHECK_RC(rc, "InitFromXmlFile");

	rc = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
	CHECK_RC(rc, "Find depth generator");
	rc = g_Context.FindExistingNode(XN_NODE_TYPE_HANDS, g_HandsGenerator);
	CHECK_RC(rc, "Find hands generator");

	// Create NITE objects
	g_pSessionManager = new XnVSessionManager;
	rc = g_pSessionManager->Initialize(&g_Context, "Click,Wave", "RaiseHand");
	CHECK_RC(rc, "SessionManager::Initialize");

	g_pSessionManager->RegisterSession(NULL, SessionStarting, SessionEnding, FocusProgress);

	g_pDrawer = new XnVPointDrawer(20, g_DepthGenerator); 
	g_pFlowRouter = new XnVFlowRouter;
	g_pFlowRouter->SetActive(g_pDrawer);

	g_pSessionManager->AddListener(g_pFlowRouter);

	g_pDrawer->RegisterNoPoints(NULL, NoHands);
	g_pDrawer->SetDepthMap(g_bDrawDepthMap);

	// Initialization done. Start generating
	rc = g_Context.StartGeneratingAll();
	CHECK_RC(rc, "StartGenerating");

	//-----------------KINECT---------------------------

	int done = 0;
	while ( !done )
	{
		SDL_GetMouseState(&state.x, &state.y);
		state.LeftButtonDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1);
		state.MiddleButtonDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(2);
		state.RightButtonDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(3);

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


