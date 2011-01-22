#ifndef GLENGINE_H
#define GLENGINE_H

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

#include "resource.h"
#include "Shader.h"

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <gl/GL.h>
	#include <gl/GLU.h>
#endif

#include <string>

#include "bitmap.h"
#include "ModelOBJ.h"

// GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

#define iGLEngine GLEngine::getEngine()
//-----------------------------------------------------------------------------
// Type definitions.
//-----------------------------------------------------------------------------

typedef std::map<std::string, GLuint> ModelTextures;


class GLEngine
{
public:
	GLEngine();
	~GLEngine();

	static GLvoid Uninitialize(GLvoid);
	static GLEngine *getEngine(GLvoid);
	
	GLuint loadTexture(const char *pszFilename);
	void loadModel(const char *pszFilename = "");

	ModelOBJ getModel(void);
	ModelOBJ::GroupObject *getObject(int index);
	
	GLvoid Initialize(GLint width, GLint height);
	GLvoid establishProjectionMatrix(GLsizei width, GLsizei height);

	bool	extensionSupported(const char *pszExtensionName);

	void	drawModelUsingProgrammablePipeline();
	void	drawModelUsingFixedFuncPipeline();
	
	GLvoid buildTextureFont(GLvoid);
	GLvoid drawText(GLint x, GLint y, const char *text, ...);

	GLuint getTextWidth(const char *text);
	GLuint getTextHeight(const char *text);

	// Returns the highest supported GL version.
	void GL2GetGLVersion(int &major, int &minor);

	// Returns the highest supported GLSL version.
	void GL2GetGLSLVersion(int &major, int &minor);

	// Determines whether the specified GL version is supported.
	bool GL2SupportsGLVersion(int major, int minor);

	// Determines whether the specified GLSL version is supported.
	bool GL2SupportsGLSLVersion(int major, int minor);

	bool	supportProgrammablePipeline(void);

	ModelOBJ::GroupObject *getSelectedItem();
	void setSelectedItem( ModelOBJ::GroupObject *selected );

	ModelOBJ::GroupObject *selectedItem;
	ModelOBJ::GroupObject *selectedSquare;

private:
	Shader				*shader;
	HWND                hWnd;
	HDC                 hDC;
	HGLRC               hRC;
	GLuint				fontBase;
	Texture				*fontTexture;
	GLuint				nullTexture;
	GLuint              blinnPhongShader;
	GLuint              normalMappingShader;

	bool				supportsProgrammablePipeline;
	int                 msaaSamples;
	float               maxAnisotrophy;
	bool                enableTextures;

	ModelOBJ            model;
	ModelTextures       modelTextures;

	GLuint				displayList;

	vector<ModelOBJ::GroupObject *> objects;

	


};


#endif