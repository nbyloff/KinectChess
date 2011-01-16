#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <gl/GL.h>
	#include <gl/GLU.h>
#endif

#include <cstdio>
#include <string>
#include <vector>
#include <fstream>

using std::string;
using std::vector;
using std::ifstream;


struct TGA_Header
{
	GLubyte		ID_Length;
	GLubyte		ColorMapType;
	GLubyte		ImageType; 
	GLubyte		colorMapSpecification[5];
	GLshort		xOrigin;
	GLshort		yOrigin;
	GLshort		ImageWidth;
	GLshort		ImageHeight;
	GLshort		PixelDepth;
};

class Texture
{
//member functions
public: 
	//pixels, width, height
	//Texture(char *filename, string name = "");
	Texture(string filename, string name = "");
	~Texture();

	int		getMode(void);

	static GLuint	createNullTexture(int width = 2, int height = 2);

	bool	loadBMP(string filename);
	bool	loadTexture(char *ps, int w = 0, int h = 0);
	string	getFileExtension(const std::string &FileName);

//variables
public:
	//char			*pixels;
	unsigned char	*imageData;
	unsigned int	bpp;
	unsigned int	width;
	unsigned int	height;
	unsigned int	texID;
	string			name;

	//keep global track of textures created
	static vector<Texture *> textures;

private:
	bool loadTGA(string filename);
	bool createTexture(unsigned char *ps, int w, int h, int type); //TODO; merge with loadTexture


};


#endif