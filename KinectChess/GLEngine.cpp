#include "GLEngine.h"
#include <stdexcept>

const int fontSize = 13;
const int fontSpace = 7;

GLEngine::GLEngine()
{
	supportsProgrammablePipeline = false;
	shader = new Shader();
	enableTextures = true;
}

GLEngine::~GLEngine()
{
	//TODO: Delete everything
}

GLvoid GLEngine::Uninitialize()
{
	GLEngine::getEngine()->getModel().destroy();
	delete GLEngine::getEngine()->shader;
	delete GLEngine::getEngine();
}

GLEngine *GLEngine::getEngine()
{
	//singleton engine
	static GLEngine *engine = new GLEngine();
	return engine;
}

ModelOBJ GLEngine::getModel(void)
{
	return model;
}

ModelOBJ::GroupObject *GLEngine::getObject(int index)
{
	return model.getObject( index );
}

GLuint GLEngine::loadTexture(const char *pszFilename)
{
    GLuint id = 0;
    Bitmap bitmap;

    if (bitmap.loadPicture(pszFilename))
    {
        // The Bitmap class loads images and orients them top-down.
        // OpenGL expects bitmap images to be oriented bottom-up.
        bitmap.flipVertical();

        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        if (maxAnisotrophy > 1.0f)
        {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                maxAnisotrophy);
        }

        gluBuild2DMipmaps(GL_TEXTURE_2D, 4, bitmap.width, bitmap.height,
            GL_BGRA_EXT, GL_UNSIGNED_BYTE, bitmap.getPixels());
    }

    return id;
}

void GLEngine::loadModel(const char *pszFilename)
{
    // Import the OBJ file and normalize to unit length.
    if (!model.import(pszFilename))
    {
        throw std::runtime_error("Failed to load model.");
    }

    // Load any associated textures.
    // Note the path where the textures are assumed to be located.
    const ModelOBJ::Material *pMaterial = 0;
    GLuint textureId = 0;
    std::string::size_type offset = 0;
    std::string filename;

    for (int i = 0; i < model.getNumberOfMaterials(); ++i)
    {
        pMaterial = &model.getMaterial(i);

        // Look for and load any diffuse color map textures.
        if (pMaterial->colorMapFilename.empty())
            continue;

        // Try load the texture using the path in the .MTL file.
		textureId = loadTexture(pMaterial->colorMapFilename.c_str());

        if (!textureId)
        {
            offset = pMaterial->colorMapFilename.find_last_of('\\');

            if (offset != std::string::npos)
                filename = pMaterial->colorMapFilename.substr(++offset);
            else
                filename = pMaterial->colorMapFilename;

            // Try loading the texture from the same directory as the OBJ file.
            textureId = loadTexture((model.getPath() + filename).c_str());
        }

        if (textureId)
            modelTextures[pMaterial->colorMapFilename] = textureId;

        // Look for and load any normal map textures.
        if (pMaterial->bumpMapFilename.empty())
            continue;

        // Try load the texture using the path in the .MTL file.
        textureId = loadTexture(pMaterial->bumpMapFilename.c_str());

        if (!textureId)
        {
            offset = pMaterial->bumpMapFilename.find_last_of('\\');

            if (offset != std::string::npos)
                filename = pMaterial->bumpMapFilename.substr(++offset);
            else
                filename = pMaterial->bumpMapFilename;

            // Try loading the texture from the same directory as the OBJ file.
            textureId = loadTexture((model.getPath() + filename).c_str());
        }

        if (textureId)
            modelTextures[pMaterial->bumpMapFilename] = textureId;
    }
}


GLvoid GLEngine::establishProjectionMatrix(GLsizei width, GLsizei height)
{
	//x, y, width, height
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	//cast to float because vars are int's
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 200.0f);
}


GLvoid GLEngine::Initialize(GLint width, GLint height)
{

	GLenum err = glewInit();
	selectedItem = NULL;
	selectedSquare = NULL;
	isObjectSelected = false;
	isSquareSelected = false;

    supportsProgrammablePipeline = GL2SupportsGLVersion(2, 0);

    // Check for GL_EXT_texture_filter_anisotropic support.
    if (extensionSupported("GL_EXT_texture_filter_anisotropic"))
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotrophy);
    else
        maxAnisotrophy = 1.0f;

	glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
	glClearStencil(0);

    if (supportsProgrammablePipeline)
    {
        std::string infoLog;

        if (!(blinnPhongShader = shader->loadShaderProgramFromResource(
            reinterpret_cast<const char *>(SHADER_BLINN_PHONG), infoLog)))
            throw std::runtime_error("Failed to load Blinn-Phong shader.\n" + infoLog);

        if (!(normalMappingShader = shader->loadShaderProgramFromResource(
            reinterpret_cast<const char *>(SHADER_NORMAL_MAPPING), infoLog)))
            throw std::runtime_error("Failed to load normal mapping shader.\n" + infoLog);

		if (!(nullTexture = Texture::createNullTexture(2, 2)))
            throw std::runtime_error("Failed to create null texture.");
    }
}

bool GLEngine::extensionSupported(const char *pszExtensionName)
{
    static const char *pszGLExtensions = 0;
    static const char *pszWGLExtensions = 0;

    if (!pszGLExtensions)
        pszGLExtensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));

    if (!pszWGLExtensions)
    {
        // WGL_ARB_extensions_string.
        typedef const char *(WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC);

        PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB =
            reinterpret_cast<PFNWGLGETEXTENSIONSSTRINGARBPROC>(
            wglGetProcAddress("wglGetExtensionsStringARB"));

        if (wglGetExtensionsStringARB)
            pszWGLExtensions = wglGetExtensionsStringARB(hDC);
    }

    if (!strstr(pszGLExtensions, pszExtensionName))
    {
        if (!strstr(pszWGLExtensions, pszExtensionName))
            return false;
    }

    return true;
}

GLvoid GLEngine::buildTextureFont()
{
	fontBase = glGenLists(256); //16 rows * 16 columns in Font.tga
	glBindTexture(GL_TEXTURE_2D, fontTexture->texID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE); //so we can put text over 3d models without black area showing up

	for (int i = 0; i < 255; i++)
	{
		//drawing text VTM 23 14 min
		float cx = (float)(i % 16) / 16.0f;
		float cy = (float)(i / 16) / 16.0f;

		glNewList(fontBase + i, GL_COMPILE);  //NOT AND_EXECUTE so it doesnt print everything

		glBegin(GL_QUADS);
			//starts at top; 1/16 columns = 0.0625				//0, size of texture
			glTexCoord2f(cx,			1 - cy - 0.0625f);	glVertex2i( 0, fontSize);
			glTexCoord2f(cx + 0.0625f,	1 - cy - 0.0625f);	glVertex2i( fontSize, fontSize);
			glTexCoord2f(cx + 0.0625f,	1 - cy);			glVertex2i( fontSize, 0);
			glTexCoord2f(cx,			1 - cy);			glVertex2i( 0, 0);
		glEnd();

		glTranslated(fontSpace, 0, 0);
		glEndList();
	}

}

GLvoid GLEngine::drawText(GLint x, GLint y, const char *in_text, ...)
{
	char text[256];

	va_list	ap;
	va_start(ap, in_text);
		vsprintf(text, in_text, ap);
	va_end(ap);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glBindTexture(GL_TEXTURE_2D, fontTexture->texID);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();

	glTranslated(x, y, 0);
	glListBase(fontBase);
	glCallLists( (GLsizei)strlen(text), GL_BYTE, text);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void GLEngine::drawModelUsingFixedFuncPipeline()
{
    const ModelOBJ::Mesh *pMesh = 0;
    const ModelOBJ::Material *pMaterial = 0;
    const ModelOBJ::Vertex *pVertices = 0;
    ModelTextures::const_iterator iter;

    for (int i = 0; i < model.getNumberOfMeshes(); ++i)
    {
        pMesh = &model.getMesh(i);
        pMaterial = pMesh->pMaterial;
        pVertices = model.getVertexBuffer();

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pMaterial->ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pMaterial->diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pMaterial->specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pMaterial->shininess * 128.0f);

        if (enableTextures)
        {
            iter = modelTextures.find(pMaterial->colorMapFilename);

            if (iter == modelTextures.end())
            {
                glDisable(GL_TEXTURE_2D);
            }
            else
            {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, iter->second);
            }
        }
        else
        {
            glDisable(GL_TEXTURE_2D);
        }

        if (model.hasPositions())
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, model.getVertexSize(),
                model.getVertexBuffer()->position);
        }

        if (model.hasTextureCoords())
        {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, model.getVertexSize(),
                model.getVertexBuffer()->texCoord);
        }

        if (model.hasNormals())
        {
            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, model.getVertexSize(),
                model.getVertexBuffer()->normal);
        }

        glDrawElements(GL_TRIANGLES, pMesh->triangleCount * 3, GL_UNSIGNED_INT,
            model.getIndexBuffer() + pMesh->startIndex);

        if (model.hasNormals())
            glDisableClientState(GL_NORMAL_ARRAY);

        if (model.hasTextureCoords())
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        if (model.hasPositions())
            glDisableClientState(GL_VERTEX_ARRAY);
    }
}
ModelOBJ::GroupObject *GLEngine::getSelectedItem()
{
	return selectedItem;
}

void GLEngine::setSelectedItem( ModelOBJ::GroupObject *selected )
{
	selectedItem = selected;
}

void GLEngine::itemSelected( bool val )
{
	isObjectSelected = val;
}

bool GLEngine::getIsItemSelected(void)
{
	return isObjectSelected;
}


ModelOBJ::GroupObject *GLEngine::getSelectedSquare()
{
	return selectedSquare;
}

void GLEngine::setSelectedSquare( ModelOBJ::GroupObject *selected )
{
	selectedSquare = selected;
}

void GLEngine::squareSelected( bool val )
{
	isSquareSelected = val;
}

bool GLEngine::getIsSquareSelected(void)
{
	return isSquareSelected;
}

void GLEngine::drawModelUsingProgrammablePipeline()
{
	ModelTextures::const_iterator iter;
    GLuint texture = 0;

	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(blinnPhongShader);
	//const ModelOBJ::Material *pMaterial = 0;

	// Stencil setup
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
	glEnable( GL_STENCIL_TEST );

	objects = model.getObjects();
	// Loop through objects...
	for( int i=0 ; i < (int)objects.size(); ++i )
	{
		ModelOBJ::GroupObject *object = objects[i];

		bool moveObject = false;
		if ( selectedItem != NULL )
		{
			//We want to color this object light green to show selection
			if ( selectedItem->index == object->index )
				moveObject = true;
		}

		// Draw to stencil buffer with object's index
		glStencilFunc( GL_ALWAYS, object->index, 0xFF );

		// Loop through materials used by object...
		for( int j=0 ; j < (int)object->materials.size() ; ++j )
		{
			ModelOBJ::Material *pMaterial = object->materials[j];

			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pMaterial->ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pMaterial->diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pMaterial->specular);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pMaterial->shininess * 128.0f);

			if (pMaterial->bumpMapFilename.empty())
			{
				//Bind the color map texture.
				texture = nullTexture;
				if (enableTextures)
				{
					iter = modelTextures.find(pMaterial->colorMapFilename);

					if (iter != modelTextures.end())
						texture = iter->second;
				}

				glActiveTexture(GL_TEXTURE0);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, texture);

				//Update shader parameters.
				glUniform1i(glGetUniformLocation(
					blinnPhongShader, "colorMap"), 0);
				glUniform1f(glGetUniformLocation(
					blinnPhongShader, "materialAlpha"), pMaterial->alpha);
				if ( moveObject )
					glUniform4f(glGetUniformLocation(
						blinnPhongShader, "Ambient"), 0.0f, 1.0f, 0.0f, 1.0f);
				else
					glUniform4f(glGetUniformLocation(
						blinnPhongShader, "Ambient"), 0.0f, 0.0f, 0.0f, 0.0f);
			}
			if (model.hasPositions())
			{
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3, GL_FLOAT, model.getVertexSize(),
					model.getVertexBuffer()->position);
			}

			if (model.hasTextureCoords())
			{
				glClientActiveTexture(GL_TEXTURE0);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(2, GL_FLOAT, model.getVertexSize(),
					model.getVertexBuffer()->texCoord);
			}

			if (model.hasNormals())
			{
				glEnableClientState(GL_NORMAL_ARRAY);
				glNormalPointer(GL_FLOAT, model.getVertexSize(),
					model.getVertexBuffer()->normal);
			}

			if ( object->position.moved )
			{
				glPushMatrix();
				glTranslatef(object->position.translate.x, 0.0f, object->position.translate.z);
			}

			glDrawElements( GL_TRIANGLES, pMaterial->triangleCount * 3, GL_UNSIGNED_INT, model.getIndexBuffer() + pMaterial->startIndex );

			if ( object->position.moved )
				glPopMatrix();

			if (model.hasNormals())
				glDisableClientState(GL_NORMAL_ARRAY);

			if (model.hasTextureCoords())
			{
				glClientActiveTexture(GL_TEXTURE0);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}

			if (model.hasPositions())
				glDisableClientState(GL_VERTEX_ARRAY);

		}
		if ( moveObject && isSquareSelected )
		{
			selectedItem = NULL; //Piece has moved; Stop drawing piece green
			selectedSquare = NULL;
			isObjectSelected = false;
			isSquareSelected = false;
		}
	}
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glDisable(GL_BLEND);
}

void GLEngine::setMovePoint(Vector3 point)
{
	moveTo = point;
}

GLuint GLEngine::getTextWidth(const char *text)
{
	return GLuint( strlen(text) * fontSpace );
}

GLuint GLEngine::getTextHeight(const char *text)
{
	return GLuint( fontSize );
}

bool GLEngine::supportProgrammablePipeline(void)
{
	return supportsProgrammablePipeline;
}

void GLEngine::GL2GetGLVersion(int &major, int &minor)
{
    static int majorGL = 0;
    static int minorGL = 0;

    if (!majorGL && !minorGL)
    {
        const char *pszVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));

        if (pszVersion)
            sscanf(pszVersion, "%d.%d", &majorGL, &minorGL);
    }

    major = majorGL;
    minor = minorGL;
}

void GLEngine::GL2GetGLSLVersion(int &major, int &minor)
{
    static int majorGLSL = 0;
    static int minorGLSL = 0;

    if (!majorGLSL && !minorGLSL)
    {
        int majorGL = 0;
        int minorGL = 0;

        GL2GetGLVersion(majorGL, minorGL);

        if (majorGL >= 2)
        {
            const char *pszShaderVersion = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));

            if (pszShaderVersion)
                sscanf(pszShaderVersion, "%d.%d", &majorGLSL, &minorGLSL);
        }
        else
        {
            const char *pszExtension = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));

            if (pszExtension)
            {
                if (strstr(pszExtension, "GL_ARB_shading_language_100"))
                {
                    majorGLSL = 1;
                    minorGLSL = 0;
                }
            }
        }
    }

    major = majorGLSL;
    minor = minorGLSL;
}

bool GLEngine::GL2SupportsGLVersion(int major, int minor)
{
    static int majorGL = 0;
    static int minorGL = 0;

    if (!majorGL && !minorGL)
        GL2GetGLVersion(majorGL, minorGL);

    if (majorGL > major)
        return true;

    if (majorGL == major && minorGL >= minor)
        return true;

    return false;
}

bool GLEngine::GL2SupportsGLSLVersion(int major, int minor)
{
    static int majorGLSL = 0;
    static int minorGLSL = 0;

    if (!majorGLSL && !minorGLSL)
        GL2GetGLSLVersion(majorGLSL, minorGLSL);

    if (majorGLSL > major)
        return true;

    if (majorGLSL == major && minorGLSL >= minor)
        return true;

    return false;
}
