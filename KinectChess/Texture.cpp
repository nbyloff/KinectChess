#include "Texture.h"
#include <assert.h>

vector<Texture *> Texture::textures;

//Texture::Texture(char *ps, int w, int h) : pixels(ps), width(w), height(h)
//Texture::Texture(char *in_filename, string in_name)
Texture::Texture(string in_filename, string in_name)
{
	imageData = NULL;
	string fileType = getFileExtension(in_filename);
	if (fileType == "bmp")
	{
		if (loadBMP( in_filename ) )
				textures.push_back(this);
	} else if (fileType == "tga") {
		if (loadTGA( in_filename ) )
				textures.push_back(this);
	}
	name = in_name;
}

Texture::~Texture()
{
	for (vector<Texture *>::iterator it = textures.begin(); it != textures.end(); it++)
	{
		if ( (*it) == this)
			textures.erase(it);
	}
	delete imageData;
	//delete pixels;
}

namespace {
	//Converts a four-character array to an integer, using little-endian form
	int toInt(const char* bytes) {
		return (int)(((unsigned char)bytes[3] << 24) |
					 ((unsigned char)bytes[2] << 16) |
					 ((unsigned char)bytes[1] << 8) |
					 (unsigned char)bytes[0]);
	}

	//Converts a two-character array to a short, using little-endian form
	short toShort(const char* bytes) {
		return (short)(((unsigned char)bytes[1] << 8) |
					   (unsigned char)bytes[0]);
	}

	//Reads the next four bytes as an integer, using little-endian form
	int readInt(ifstream &input) {
		char buffer[4];
		input.read(buffer, 4);
		return toInt(buffer);
	}

	//Reads the next two bytes as a short, using little-endian form
	short readShort(ifstream &input) {
		char buffer[2];
		input.read(buffer, 2);
		return toShort(buffer);
	}

	//Just like auto_ptr, but for arrays
	template<class T>
	class auto_array {
		private:
			T* array;
			mutable bool isReleased;
		public:
			explicit auto_array(T* array_ = NULL) :
				array(array_), isReleased(false) {
			}

			auto_array(const auto_array<T> &aarray) {
				array = aarray.array;
				isReleased = aarray.isReleased;
				aarray.isReleased = true;
			}

			~auto_array() {
				if (!isReleased && array != NULL) {
					delete[] array;
				}
			}

			T* get() const {
				return array;
			}

			T &operator*() const {
				return *array;
			}

			void operator=(const auto_array<T> &aarray) {
				if (!isReleased && array != NULL) {
					delete[] array;
				}
				array = aarray.array;
				isReleased = aarray.isReleased;
				aarray.isReleased = true;
			}

			T* operator->() const {
				return array;
			}

			T* release() {
				isReleased = true;
				return array;
			}

			void reset(T* array_ = NULL) {
				if (!isReleased && array != NULL) {
					delete[] array;
				}
				array = array_;
			}

			T* operator+(int i) {
				return array + i;
			}

			T &operator[](int i) {
				return array[i];
			}
	};
}

//Texture* loadBMP(const char* filename) {
bool Texture::loadBMP(string filename) {
	ifstream input;
	input.open(filename.c_str());
	//input.open(filename, ifstream::binary);
	assert(!input.fail() || !"Could not find file");
	char buffer[2];
	input.read(buffer, 2);
	assert(buffer[0] == 'B' && buffer[1] == 'M' || !"Not a bitmap file");
	input.ignore(8);
	int dataOffset = readInt(input);

	//Read the header
	int headerSize = readInt(input);
	int width;
	int height;
	switch(headerSize) {
		case 40:
			//V3
			width = readInt(input);
			height = readInt(input);
			input.ignore(2);
			assert(readShort(input) == 24 || !"Image is not 24 bits per pixel");
			assert(readShort(input) == 0 || !"Image is compressed");
			break;
		case 12:
			//OS/2 V1
			width = readShort(input);
			height = readShort(input);
			input.ignore(2);
			assert(readShort(input) == 24 || !"Image is not 24 bits per pixel");
			break;
		case 64:
			//OS/2 V2
			assert(!"Can't load OS/2 V2 bitmaps");
			break;
		case 108:
			//Windows V4
			assert(!"Can't load Windows V4 bitmaps");
			break;
		case 124:
			//Windows V5
			assert(!"Can't load Windows V5 bitmaps");
			break;
		default:
			assert(!"Unknown bitmap format");
	}

	//Read the data
	int bytesPerRow = ((width * 3 + 3) / 4) * 4 - (width * 3 % 4);
	int size = bytesPerRow * height;
	auto_array<char> pixels(new char[size]);
	input.seekg(dataOffset, std::ios_base::beg);
	input.read(pixels.get(), size);

	//Get the data into the right format
	auto_array<char> pixels2(new char[width * height * 3]);
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			for(int c = 0; c < 3; c++) {
				pixels2[3 * (width * y + x) + c] =
					pixels[bytesPerRow * y + 3 * x + (2 - c)];
			}
		}
	}

	input.close();
	//pixels = pixels2;
	loadTexture(pixels2.release(), width, height);
	return true;
	//return new Texture(pixels2.release(), width, height);
}

GLuint Texture::createNullTexture(int width, int height)
{
    // Create an empty white texture. This texture is applied to OBJ models
    // that don't have any texture maps. This trick allows the same shader to
    // be used to draw the OBJ model with and without textures applied.

    int pitch = ((width * 32 + 31) & ~31) >> 3; // align to 4-byte boundaries
    std::vector<GLubyte> pixels(pitch * height, 255);
    GLuint texture = 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, &pixels[0]);

    return texture;
}

bool Texture::loadTGA(string fileName)
{
	TGA_Header TGAheader;

	ifstream file( fileName.data(), std::ios_base::binary);

	if (!file.is_open() )
		return false;

	if ( !file.read( (char *)&TGAheader, sizeof(TGAheader) ) )
		return false;

	//if ( (int)TGAheader.ImageType != 2)
	//	return false;

	width = TGAheader.ImageWidth;
	height = TGAheader.ImageHeight;
	bpp = TGAheader.PixelDepth;

	if (width <= 0 || height <= 0 || (bpp != 24 && bpp != 32))
		return false;

	GLuint type = GL_RGBA;

	if (bpp == 24)
		type = GL_RGB; //don't have extra channel for alpha

	GLuint bytesPerPixel = bpp / 8;
	GLuint imageSize = width * height * bytesPerPixel;

	imageData = new GLubyte[imageSize];

	if (imageData == NULL)
		return false; //out of memory?

	if (!file.read( (char *)imageData, imageSize ) )
	{
		delete imageData; //something went wrong, give memory back
		return false;
	}

	//BGR --> RGB
	for (GLuint i = 0; i < (int)imageSize; i += bytesPerPixel )
	{
		GLuint temp = imageData[i];
		imageData[i] = imageData[i + 2];
		imageData[i + 2] = temp;
	}

	loadTexture((char *)imageData, width, height);

	//everything worked
	return true;

}

bool Texture::createTexture(unsigned char *ps, int w, int h, int type)
{
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	return true;
}

//Makes the image into a texture, and returns the id of the texture
//bool Texture::loadTexture(Texture *image) {
bool Texture::loadTexture(char *ps, int w, int h) {

	glGenTextures(1, &texID); //Make room for our texture
	glBindTexture(GL_TEXTURE_2D, texID); //Tell OpenGL which texture to edit

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//Map the image to the texture
	glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
				 0,                            //0 for now
				 GL_RGB,            //Format OpenGL uses for image
				 w, h,				//Width and height
				 0,                 //The border of the image
				 GL_RGB,			//GL_RGB, because pixels are stored in RGB format
				 GL_UNSIGNED_BYTE,  //GL_UNSIGNED_BYTE, because pixels are stored
				                   //as unsigned numbers
				 ps);               //The actual pixel data

	return true;
}

std::string Texture::getFileExtension(const std::string &FileName)
{
    if(FileName.find_last_of(".") != std::string::npos)
        return FileName.substr(FileName.find_last_of(".")+1);
    return "";
}




