#include "GLEngine.h"

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

}

GLvoid GLEngine::Uninitialize(GLvoid)
{
	delete GLEngine::getEngine();
}

GLEngine *GLEngine::getEngine(GLvoid)
{
	//singleton engine
	static GLEngine *engine = new GLEngine();
	return engine;
}

ModelOBJ GLEngine::getModel(void)
{
	return model;
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

    model.normalize();
	
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


void GLEngine::GL2Init()
{
    #define GPA(x) wglGetProcAddress(x)

    // OpenGL 1.2.
    glCopyTexSubImage3D         = reinterpret_cast<PFNGLCOPYTEXSUBIMAGE3DPROC>(GPA("glCopyTexSubImage3D"));
    glDrawRangeElements         = reinterpret_cast<PFNGLDRAWRANGEELEMENTSPROC>(GPA("glDrawRangeElements"));
    glTexImage3D                = reinterpret_cast<PFNGLTEXIMAGE3DPROC>(GPA("glTexImage3D"));
    glTexSubImage3D             = reinterpret_cast<PFNGLTEXSUBIMAGE3DPROC>(GPA("glTexSubImage3D"));

    // OpenGL 1.3.
    glActiveTexture             = reinterpret_cast<PFNGLACTIVETEXTUREPROC>(GPA("glActiveTexture"));
    glClientActiveTexture       = reinterpret_cast<PFNGLCLIENTACTIVETEXTUREPROC>(GPA("glClientActiveTexture"));
    glMultiTexCoord1d           = reinterpret_cast<PFNGLMULTITEXCOORD1DPROC>(GPA("glMultiTexCoord1d"));
    glMultiTexCoord1dv          = reinterpret_cast<PFNGLMULTITEXCOORD1DVPROC>(GPA("glMultiTexCoord1dv"));
    glMultiTexCoord1f           = reinterpret_cast<PFNGLMULTITEXCOORD1FPROC>(GPA("glMultiTexCoord1f"));
    glMultiTexCoord1fv          = reinterpret_cast<PFNGLMULTITEXCOORD1FVPROC>(GPA("glMultiTexCoord1fv"));
    glMultiTexCoord1i           = reinterpret_cast<PFNGLMULTITEXCOORD1IPROC>(GPA("glMultiTexCoord1i"));
    glMultiTexCoord1iv          = reinterpret_cast<PFNGLMULTITEXCOORD1IVPROC>(GPA("glMultiTexCoord1iv"));
    glMultiTexCoord1s           = reinterpret_cast<PFNGLMULTITEXCOORD1SPROC>(GPA("glMultiTexCoord1s"));
    glMultiTexCoord1sv          = reinterpret_cast<PFNGLMULTITEXCOORD1SVPROC>(GPA("glMultiTexCoord1sv"));
    glMultiTexCoord2d           = reinterpret_cast<PFNGLMULTITEXCOORD2DPROC>(GPA("glMultiTexCoord2d"));
    glMultiTexCoord2dv          = reinterpret_cast<PFNGLMULTITEXCOORD2DVPROC>(GPA("glMultiTexCoord2dv"));
    glMultiTexCoord2f           = reinterpret_cast<PFNGLMULTITEXCOORD2FPROC>(GPA("glMultiTexCoord2f"));
    glMultiTexCoord2fv          = reinterpret_cast<PFNGLMULTITEXCOORD2FVPROC>(GPA("glMultiTexCoord2fv"));
    glMultiTexCoord2i           = reinterpret_cast<PFNGLMULTITEXCOORD2IPROC>(GPA("glMultiTexCoord2i"));
    glMultiTexCoord2iv          = reinterpret_cast<PFNGLMULTITEXCOORD2IVPROC>(GPA("glMultiTexCoord2iv"));
    glMultiTexCoord2s           = reinterpret_cast<PFNGLMULTITEXCOORD2SPROC>(GPA("glMultiTexCoord2s"));
    glMultiTexCoord2sv          = reinterpret_cast<PFNGLMULTITEXCOORD2SVPROC>(GPA("glMultiTexCoord2sv"));
    glMultiTexCoord3d           = reinterpret_cast<PFNGLMULTITEXCOORD3DPROC>(GPA("glMultiTexCoord3d"));
    glMultiTexCoord3dv          = reinterpret_cast<PFNGLMULTITEXCOORD3DVPROC>(GPA("glMultiTexCoord3dv"));
    glMultiTexCoord3f           = reinterpret_cast<PFNGLMULTITEXCOORD3FPROC>(GPA("glMultiTexCoord3f"));
    glMultiTexCoord3fv          = reinterpret_cast<PFNGLMULTITEXCOORD3FVPROC>(GPA("glMultiTexCoord3fv"));
    glMultiTexCoord3i           = reinterpret_cast<PFNGLMULTITEXCOORD3IPROC>(GPA("glMultiTexCoord3i"));
    glMultiTexCoord3iv          = reinterpret_cast<PFNGLMULTITEXCOORD3IVPROC>(GPA("glMultiTexCoord3iv"));
    glMultiTexCoord3s           = reinterpret_cast<PFNGLMULTITEXCOORD3SPROC>(GPA("glMultiTexCoord3s"));
    glMultiTexCoord3sv          = reinterpret_cast<PFNGLMULTITEXCOORD3SVPROC>(GPA("glMultiTexCoord3sv"));
    glMultiTexCoord4d           = reinterpret_cast<PFNGLMULTITEXCOORD4DPROC>(GPA("glMultiTexCoord4d"));
    glMultiTexCoord4dv          = reinterpret_cast<PFNGLMULTITEXCOORD4DVPROC>(GPA("glMultiTexCoord4dv"));
    glMultiTexCoord4f           = reinterpret_cast<PFNGLMULTITEXCOORD4FPROC>(GPA("glMultiTexCoord4f"));
    glMultiTexCoord4fv          = reinterpret_cast<PFNGLMULTITEXCOORD4FVPROC>(GPA("glMultiTexCoord4fv"));
    glMultiTexCoord4i           = reinterpret_cast<PFNGLMULTITEXCOORD4IPROC>(GPA("glMultiTexCoord4i"));
    glMultiTexCoord4iv          = reinterpret_cast<PFNGLMULTITEXCOORD4IVPROC>(GPA("glMultiTexCoord4iv"));
    glMultiTexCoord4s           = reinterpret_cast<PFNGLMULTITEXCOORD4SPROC>(GPA("glMultiTexCoord4s"));
    glMultiTexCoord4sv          = reinterpret_cast<PFNGLMULTITEXCOORD4SVPROC>(GPA("glMultiTexCoord4sv"));
    glLoadTransposeMatrixf      = reinterpret_cast<PFNGLLOADTRANSPOSEMATRIXFPROC>(GPA("glLoadTransposeMatrixf"));
    glLoadTransposeMatrixd      = reinterpret_cast<PFNGLLOADTRANSPOSEMATRIXDPROC>(GPA("glLoadTransposeMatrixd"));
    glMultTransposeMatrixf      = reinterpret_cast<PFNGLMULTTRANSPOSEMATRIXFPROC>(GPA("glMultTransposeMatrixf"));
    glMultTransposeMatrixd      = reinterpret_cast<PFNGLMULTTRANSPOSEMATRIXDPROC>(GPA("glMultTransposeMatrixd"));
    glSampleCoverage            = reinterpret_cast<PFNGLSAMPLECOVERAGEPROC>(GPA("glSampleCoverage"));
    glCompressedTexImage3D      = reinterpret_cast<PFNGLCOMPRESSEDTEXIMAGE3DPROC>(GPA("glCompressedTexImage3D"));
    glCompressedTexImage2D      = reinterpret_cast<PFNGLCOMPRESSEDTEXIMAGE2DPROC>(GPA("glCompressedTexImage2D"));
    glCompressedTexImage1D      = reinterpret_cast<PFNGLCOMPRESSEDTEXIMAGE1DPROC>(GPA("glCompressedTexImage1D"));
    glCompressedTexSubImage3D   = reinterpret_cast<PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC>(GPA("glCompressedTexSubImage3D"));
    glCompressedTexSubImage2D   = reinterpret_cast<PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC>(GPA("glCompressedTexSubImage2D"));
    glCompressedTexSubImage1D   = reinterpret_cast<PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC>(GPA("glCompressedTexSubImage1D"));
    glGetCompressedTexImage     = reinterpret_cast<PFNGLGETCOMPRESSEDTEXIMAGEPROC>(GPA("glGetCompressedTexImage"));

    // OpenGL 1.4.
    glMultiDrawArrays           = reinterpret_cast<PFNGLMULTIDRAWARRAYSPROC>(GPA("glMultiDrawArrays"));
    glMultiDrawElements         = reinterpret_cast<PFNGLMULTIDRAWELEMENTSPROC>(GPA("glMultiDrawElements"));
    glPointParameterf           = reinterpret_cast<PFNGLPOINTPARAMETERFPROC>(GPA("glPointParameterf"));
    glPointParameterfv          = reinterpret_cast<PFNGLPOINTPARAMETERFVPROC>(GPA("glPointParameterfv"));
    glSecondaryColor3b          = reinterpret_cast<PFNGLSECONDARYCOLOR3BPROC>(GPA("glSecondaryColor3b"));
    glSecondaryColor3bv         = reinterpret_cast<PFNGLSECONDARYCOLOR3BVPROC>(GPA("glSecondaryColor3bv"));
    glSecondaryColor3d          = reinterpret_cast<PFNGLSECONDARYCOLOR3DPROC>(GPA("glSecondaryColor3d"));
    glSecondaryColor3dv         = reinterpret_cast<PFNGLSECONDARYCOLOR3DVPROC>(GPA("glSecondaryColor3dv"));
    glSecondaryColor3f          = reinterpret_cast<PFNGLSECONDARYCOLOR3FPROC>(GPA("glSecondaryColor3f"));
    glSecondaryColor3fv         = reinterpret_cast<PFNGLSECONDARYCOLOR3FVPROC>(GPA("glSecondaryColor3fv"));
    glSecondaryColor3i          = reinterpret_cast<PFNGLSECONDARYCOLOR3IPROC>(GPA("glSecondaryColor3i"));
    glSecondaryColor3iv         = reinterpret_cast<PFNGLSECONDARYCOLOR3IVPROC>(GPA("glSecondaryColor3iv"));
    glSecondaryColor3s          = reinterpret_cast<PFNGLSECONDARYCOLOR3SPROC>(GPA("glSecondaryColor3s"));
    glSecondaryColor3sv         = reinterpret_cast<PFNGLSECONDARYCOLOR3SVPROC>(GPA("glSecondaryColor3sv"));
    glSecondaryColor3ub         = reinterpret_cast<PFNGLSECONDARYCOLOR3UBPROC>(GPA("glSecondaryColor3ub"));
    glSecondaryColor3ubv        = reinterpret_cast<PFNGLSECONDARYCOLOR3UBVPROC>(GPA("glSecondaryColor3ubv"));
    glSecondaryColor3ui         = reinterpret_cast<PFNGLSECONDARYCOLOR3UIPROC>(GPA("glSecondaryColor3ui"));
    glSecondaryColor3uiv        = reinterpret_cast<PFNGLSECONDARYCOLOR3UIVPROC>(GPA("glSecondaryColor3uiv"));
    glSecondaryColor3us         = reinterpret_cast<PFNGLSECONDARYCOLOR3USPROC>(GPA("glSecondaryColor3us"));
    glSecondaryColor3usv        = reinterpret_cast<PFNGLSECONDARYCOLOR3USVPROC>(GPA("glSecondaryColor3usv"));
    glSecondaryColorPointer     = reinterpret_cast<PFNGLSECONDARYCOLORPOINTERPROC>(GPA("glSecondaryColorPointer"));
    glBlendFuncSeparate         = reinterpret_cast<PFNGLBLENDFUNCSEPARATEPROC>(GPA("glBlendFuncSeparate"));
    glWindowPos2d               = reinterpret_cast<PFNGLWINDOWPOS2DPROC>(GPA("glWindowPos2d"));
    glWindowPos2f               = reinterpret_cast<PFNGLWINDOWPOS2FPROC>(GPA("glWindowPos2f"));
    glWindowPos2i               = reinterpret_cast<PFNGLWINDOWPOS2IPROC>(GPA("glWindowPos2i"));
    glWindowPos2s               = reinterpret_cast<PFNGLWINDOWPOS2SPROC>(GPA("glWindowPos2s"));
    glWindowPos2dv              = reinterpret_cast<PFNGLWINDOWPOS2DVPROC>(GPA("glWindowPos2dv"));
    glWindowPos2fv              = reinterpret_cast<PFNGLWINDOWPOS2FVPROC>(GPA("glWindowPos2fv"));
    glWindowPos2iv              = reinterpret_cast<PFNGLWINDOWPOS2IVPROC>(GPA("glWindowPos2iv"));
    glWindowPos2sv              = reinterpret_cast<PFNGLWINDOWPOS2SVPROC>(GPA("glWindowPos2sv"));
    glWindowPos3d               = reinterpret_cast<PFNGLWINDOWPOS3DPROC>(GPA("glWindowPos3d"));
    glWindowPos3f               = reinterpret_cast<PFNGLWINDOWPOS3FPROC>(GPA("glWindowPos3f"));
    glWindowPos3i               = reinterpret_cast<PFNGLWINDOWPOS3IPROC>(GPA("glWindowPos3i"));
    glWindowPos3s               = reinterpret_cast<PFNGLWINDOWPOS3SPROC>(GPA("glWindowPos3s"));
    glWindowPos3dv              = reinterpret_cast<PFNGLWINDOWPOS3DVPROC>(GPA("glWindowPos3dv"));
    glWindowPos3fv              = reinterpret_cast<PFNGLWINDOWPOS3FVPROC>(GPA("glWindowPos3fv"));
    glWindowPos3iv              = reinterpret_cast<PFNGLWINDOWPOS3IVPROC>(GPA("glWindowPos3iv"));
    glWindowPos3sv              = reinterpret_cast<PFNGLWINDOWPOS3SVPROC>(GPA("glWindowPos3sv"));

    // OpenGL 1.5.
    glGenQueries                = reinterpret_cast<PFNGLGENQUERIESPROC>(GPA("glGenQueries"));
    glDeleteQueries             = reinterpret_cast<PFNGLDELETEQUERIESPROC>(GPA("glDeleteQueries"));
    glIsQuery                   = reinterpret_cast<PFNGLISQUERYPROC>(GPA("glIsQuery"));
    glBeginQuery                = reinterpret_cast<PFNGLBEGINQUERYPROC>(GPA("glBeginQuery"));
    glEndQuery                  = reinterpret_cast<PFNGLENDQUERYPROC>(GPA("glEndQuery"));
    glGetQueryiv                = reinterpret_cast<PFNGLGETQUERYIVPROC>(GPA("glGetQueryiv"));
    glGetQueryObjectiv          = reinterpret_cast<PFNGLGETQUERYOBJECTIVPROC>(GPA("glGetQueryObjectiv"));
    glGetQueryObjectuiv         = reinterpret_cast<PFNGLGETQUERYOBJECTUIVPROC>(GPA("glGetQueryObjectuiv"));
    glBindBuffer                = reinterpret_cast<PFNGLBINDBUFFERPROC>(GPA("glBindBuffer"));
    glDeleteBuffers             = reinterpret_cast<PFNGLDELETEBUFFERSPROC>(GPA("glDeleteBuffers"));
    glGenBuffers                = reinterpret_cast<PFNGLGENBUFFERSPROC>(GPA("glGenBuffers"));
    glIsBuffer                  = reinterpret_cast<PFNGLISBUFFERPROC>(GPA("glIsBuffer"));
    glBufferData                = reinterpret_cast<PFNGLBUFFERDATAPROC>(GPA("glBufferData"));
    glBufferSubData             = reinterpret_cast<PFNGLBUFFERSUBDATAPROC>(GPA("glBufferSubData"));
    glGetBufferSubData          = reinterpret_cast<PFNGLGETBUFFERSUBDATAPROC>(GPA("glGetBufferSubData"));
    glMapBuffer                 = reinterpret_cast<PFNGLMAPBUFFERPROC>(GPA("glMapBuffer"));
    glUnmapBuffer               = reinterpret_cast<PFNGLUNMAPBUFFERPROC>(GPA("glUnmapBuffer"));
    glGetBufferParameteriv      = reinterpret_cast<PFNGLGETBUFFERPARAMETERIVPROC>(GPA("glGetBufferParameteriv"));
    glGetBufferPointerv         = reinterpret_cast<PFNGLGETBUFFERPOINTERVPROC>(GPA("glGetBufferPointerv"));

    // OpenGL 2.0.
    glBlendEquationSeparate     = reinterpret_cast<PFNGLBLENDEQUATIONSEPARATEPROC>(GPA("glBlendEquationSeparate"));
    glDrawBuffers               = reinterpret_cast<PFNGLDRAWBUFFERSPROC>(GPA("glDrawBuffers"));
    glStencilOpSeparate         = reinterpret_cast<PFNGLSTENCILOPSEPARATEPROC>(GPA("glStencilOpSeparate"));
    glStencilFuncSeparate       = reinterpret_cast<PFNGLSTENCILFUNCSEPARATEPROC>(GPA("glStencilFuncSeparate"));
    glStencilMaskSeparate       = reinterpret_cast<PFNGLSTENCILMASKSEPARATEPROC>(GPA("glStencilMaskSeparate"));
    glAttachShader              = reinterpret_cast<PFNGLATTACHSHADERPROC>(GPA("glAttachShader"));
    glBindAttribLocation        = reinterpret_cast<PFNGLBINDATTRIBLOCATIONPROC>(GPA("glBindAttribLocation"));
    glCompileShader             = reinterpret_cast<PFNGLCOMPILESHADERPROC>(GPA("glCompileShader"));
    glCreateProgram             = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(GPA("glCreateProgram"));
    glCreateShader              = reinterpret_cast<PFNGLCREATESHADERPROC>(GPA("glCreateShader"));
    glDeleteProgram             = reinterpret_cast<PFNGLDELETEPROGRAMPROC>(GPA("glDeleteProgram"));
    glDeleteShader              = reinterpret_cast<PFNGLDELETESHADERPROC>(GPA("glDeleteShader"));
    glDetachShader              = reinterpret_cast<PFNGLDETACHSHADERPROC>(GPA("glDetachShader"));
    glDisableVertexAttribArray  = reinterpret_cast<PFNGLDISABLEVERTEXATTRIBARRAYPROC>(GPA("glDisableVertexAttribArray"));
    glEnableVertexAttribArray   = reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(GPA("glEnableVertexAttribArray"));
    glGetActiveAttrib           = reinterpret_cast<PFNGLGETACTIVEATTRIBPROC>(GPA("glGetActiveAttrib"));
    glGetActiveUniform          = reinterpret_cast<PFNGLGETACTIVEUNIFORMPROC>(GPA("glGetActiveUniform"));
    glGetAttachedShaders        = reinterpret_cast<PFNGLGETATTACHEDSHADERSPROC>(GPA("glGetAttachedShaders"));
    glGetAttribLocation         = reinterpret_cast<PFNGLGETATTRIBLOCATIONPROC>(GPA("glGetAttribLocation"));
    glGetProgramiv              = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(GPA("glGetProgramiv"));
    glGetProgramInfoLog         = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(GPA("glGetProgramInfoLog"));
    glGetShaderiv               = reinterpret_cast<PFNGLGETSHADERIVPROC>(GPA("glGetShaderiv"));
    glGetShaderInfoLog          = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(GPA("glGetShaderInfoLog"));
    glGetShaderSource           = reinterpret_cast<PFNGLGETSHADERSOURCEPROC>(GPA("glGetShaderSource"));
    glGetUniformLocation        = reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(GPA("glGetUniformLocation"));
    glGetUniformfv              = reinterpret_cast<PFNGLGETUNIFORMFVPROC>(GPA("glGetUniformfv"));
    glGetUniformiv              = reinterpret_cast<PFNGLGETUNIFORMIVPROC>(GPA("glGetUniformiv"));
    glGetVertexAttribdv         = reinterpret_cast<PFNGLGETVERTEXATTRIBDVPROC>(GPA("glGetVertexAttribdv"));
    glGetVertexAttribfv         = reinterpret_cast<PFNGLGETVERTEXATTRIBFVPROC>(GPA("glGetVertexAttribfv"));
    glGetVertexAttribiv         = reinterpret_cast<PFNGLGETVERTEXATTRIBIVPROC>(GPA("glGetVertexAttribiv"));
    glGetVertexAttribPointerv   = reinterpret_cast<PFNGLGETVERTEXATTRIBPOINTERVPROC>(GPA("glGetVertexAttribPointerv"));
    glIsProgram                 = reinterpret_cast<PFNGLISPROGRAMPROC>(GPA("glIsProgram"));
    glIsShader                  = reinterpret_cast<PFNGLISSHADERPROC>(GPA("glIsShader"));
    glLinkProgram               = reinterpret_cast<PFNGLLINKPROGRAMPROC>(GPA("glLinkProgram"));
    glShaderSource              = reinterpret_cast<PFNGLSHADERSOURCEPROC>(GPA("glShaderSource"));
    glUseProgram                = reinterpret_cast<PFNGLUSEPROGRAMPROC>(GPA("glUseProgram"));
    glUniform1f                 = reinterpret_cast<PFNGLUNIFORM1FPROC>(GPA("glUniform1f"));
    glUniform2f                 = reinterpret_cast<PFNGLUNIFORM2FPROC>(GPA("glUniform2f"));
    glUniform3f                 = reinterpret_cast<PFNGLUNIFORM3FPROC>(GPA("glUniform3f"));
    glUniform4f                 = reinterpret_cast<PFNGLUNIFORM4FPROC>(GPA("glUniform4f"));
    glUniform1i                 = reinterpret_cast<PFNGLUNIFORM1IPROC>(GPA("glUniform1i"));
    glUniform2i                 = reinterpret_cast<PFNGLUNIFORM2IPROC>(GPA("glUniform2i"));
    glUniform3i                 = reinterpret_cast<PFNGLUNIFORM3IPROC>(GPA("glUniform3i"));
    glUniform4i                 = reinterpret_cast<PFNGLUNIFORM4IPROC>(GPA("glUniform4i"));
    glUniform1fv                = reinterpret_cast<PFNGLUNIFORM1FVPROC>(GPA("glUniform1fv"));
    glUniform2fv                = reinterpret_cast<PFNGLUNIFORM2FVPROC>(GPA("glUniform2fv"));
    glUniform3fv                = reinterpret_cast<PFNGLUNIFORM3FVPROC>(GPA("glUniform3fv"));
    glUniform4fv                = reinterpret_cast<PFNGLUNIFORM4FVPROC>(GPA("glUniform4fv"));
    glUniform1iv                = reinterpret_cast<PFNGLUNIFORM1IVPROC>(GPA("glUniform1iv"));
    glUniform2iv                = reinterpret_cast<PFNGLUNIFORM2IVPROC>(GPA("glUniform2iv"));
    glUniform3iv                = reinterpret_cast<PFNGLUNIFORM3IVPROC>(GPA("glUniform3iv"));
    glUniform4iv                = reinterpret_cast<PFNGLUNIFORM4IVPROC>(GPA("glUniform4iv"));
    glUniformMatrix2fv          = reinterpret_cast<PFNGLUNIFORMMATRIX2FVPROC>(GPA("glUniformMatrix2fv"));
    glUniformMatrix3fv          = reinterpret_cast<PFNGLUNIFORMMATRIX3FVPROC>(GPA("glUniformMatrix3fv"));
    glUniformMatrix4fv          = reinterpret_cast<PFNGLUNIFORMMATRIX4FVPROC>(GPA("glUniformMatrix4fv"));
    glValidateProgram           = reinterpret_cast<PFNGLVALIDATEPROGRAMPROC>(GPA("glValidateProgram"));
    glVertexAttrib1d            = reinterpret_cast<PFNGLVERTEXATTRIB1DPROC>(GPA("glVertexAttrib1d"));
    glVertexAttrib1dv           = reinterpret_cast<PFNGLVERTEXATTRIB1DVPROC>(GPA("glVertexAttrib1dv"));
    glVertexAttrib1f            = reinterpret_cast<PFNGLVERTEXATTRIB1FPROC>(GPA("glVertexAttrib1f"));
    glVertexAttrib1fv           = reinterpret_cast<PFNGLVERTEXATTRIB1FVPROC>(GPA("glVertexAttrib1fv"));
    glVertexAttrib1s            = reinterpret_cast<PFNGLVERTEXATTRIB1SPROC>(GPA("glVertexAttrib1s"));
    glVertexAttrib1sv           = reinterpret_cast<PFNGLVERTEXATTRIB1SVPROC>(GPA("glVertexAttrib1sv"));
    glVertexAttrib2d            = reinterpret_cast<PFNGLVERTEXATTRIB2DPROC>(GPA("glVertexAttrib2d"));
    glVertexAttrib2dv           = reinterpret_cast<PFNGLVERTEXATTRIB2DVPROC>(GPA("glVertexAttrib2dv"));
    glVertexAttrib2f            = reinterpret_cast<PFNGLVERTEXATTRIB2FPROC>(GPA("glVertexAttrib2f"));
    glVertexAttrib2fv           = reinterpret_cast<PFNGLVERTEXATTRIB2FVPROC>(GPA("glVertexAttrib2fv"));
    glVertexAttrib2s            = reinterpret_cast<PFNGLVERTEXATTRIB2SPROC>(GPA("glVertexAttrib2s"));
    glVertexAttrib2sv           = reinterpret_cast<PFNGLVERTEXATTRIB2SVPROC>(GPA("glVertexAttrib2sv"));
    glVertexAttrib3d            = reinterpret_cast<PFNGLVERTEXATTRIB3DPROC>(GPA("glVertexAttrib3d"));
    glVertexAttrib3dv           = reinterpret_cast<PFNGLVERTEXATTRIB3DVPROC>(GPA("glVertexAttrib3dv"));
    glVertexAttrib3f            = reinterpret_cast<PFNGLVERTEXATTRIB3FPROC>(GPA("glVertexAttrib3f"));
    glVertexAttrib3fv           = reinterpret_cast<PFNGLVERTEXATTRIB3FVPROC>(GPA("glVertexAttrib3fv"));
    glVertexAttrib3s            = reinterpret_cast<PFNGLVERTEXATTRIB3SPROC>(GPA("glVertexAttrib3s"));
    glVertexAttrib3sv           = reinterpret_cast<PFNGLVERTEXATTRIB3SVPROC>(GPA("glVertexAttrib3sv"));
    glVertexAttrib4Nbv          = reinterpret_cast<PFNGLVERTEXATTRIB4NBVPROC>(GPA("glVertexAttrib4Nbv"));
    glVertexAttrib4Niv          = reinterpret_cast<PFNGLVERTEXATTRIB4NIVPROC>(GPA("glVertexAttrib4Niv"));
    glVertexAttrib4Nsv          = reinterpret_cast<PFNGLVERTEXATTRIB4NSVPROC>(GPA("glVertexAttrib4Nsv"));
    glVertexAttrib4Nub          = reinterpret_cast<PFNGLVERTEXATTRIB4NUBPROC>(GPA("glVertexAttrib4Nub"));
    glVertexAttrib4Nubv         = reinterpret_cast<PFNGLVERTEXATTRIB4NUBVPROC>(GPA("glVertexAttrib4Nubv"));
    glVertexAttrib4Nuiv         = reinterpret_cast<PFNGLVERTEXATTRIB4NUIVPROC>(GPA("glVertexAttrib4Nuiv"));
    glVertexAttrib4Nusv         = reinterpret_cast<PFNGLVERTEXATTRIB4NUSVPROC>(GPA("glVertexAttrib4Nusv"));
    glVertexAttrib4bv           = reinterpret_cast<PFNGLVERTEXATTRIB4BVPROC>(GPA("glVertexAttrib4bv"));
    glVertexAttrib4d            = reinterpret_cast<PFNGLVERTEXATTRIB4DPROC>(GPA("glVertexAttrib4d"));
    glVertexAttrib4dv           = reinterpret_cast<PFNGLVERTEXATTRIB4DVPROC>(GPA("glVertexAttrib4dv"));
    glVertexAttrib4f            = reinterpret_cast<PFNGLVERTEXATTRIB4FPROC>(GPA("glVertexAttrib4f"));
    glVertexAttrib4fv           = reinterpret_cast<PFNGLVERTEXATTRIB4FVPROC>(GPA("glVertexAttrib4fv"));
    glVertexAttrib4iv           = reinterpret_cast<PFNGLVERTEXATTRIB4IVPROC>(GPA("glVertexAttrib4iv"));
    glVertexAttrib4s            = reinterpret_cast<PFNGLVERTEXATTRIB4SPROC>(GPA("glVertexAttrib4s"));
    glVertexAttrib4sv           = reinterpret_cast<PFNGLVERTEXATTRIB4SVPROC>(GPA("glVertexAttrib4sv"));
    glVertexAttrib4ubv          = reinterpret_cast<PFNGLVERTEXATTRIB4UBVPROC>(GPA("glVertexAttrib4ubv"));
    glVertexAttrib4uiv          = reinterpret_cast<PFNGLVERTEXATTRIB4UIVPROC>(GPA("glVertexAttrib4uiv"));
    glVertexAttrib4usv          = reinterpret_cast<PFNGLVERTEXATTRIB4USVPROC>(GPA("glVertexAttrib4usv"));
    glVertexAttribPointer       = reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(GPA("glVertexAttribPointer"));

    // OpenGL 2.1.
    glUniformMatrix2x3fv        = reinterpret_cast<PFNGLUNIFORMMATRIX2X3FVPROC>(GPA("glUniformMatrix2x3fv"));
    glUniformMatrix3x2fv        = reinterpret_cast<PFNGLUNIFORMMATRIX3X2FVPROC>(GPA("glUniformMatrix3x2fv"));
    glUniformMatrix2x4fv        = reinterpret_cast<PFNGLUNIFORMMATRIX2X4FVPROC>(GPA("glUniformMatrix2x4fv"));
    glUniformMatrix4x2fv        = reinterpret_cast<PFNGLUNIFORMMATRIX4X2FVPROC>(GPA("glUniformMatrix4x2fv"));
    glUniformMatrix3x4fv        = reinterpret_cast<PFNGLUNIFORMMATRIX3X4FVPROC>(GPA("glUniformMatrix3x4fv"));
    glUniformMatrix4x3fv        = reinterpret_cast<PFNGLUNIFORMMATRIX4X3FVPROC>(GPA("glUniformMatrix4x3fv"));

    #undef GPA
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

    GL2Init();

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
	//glClearStencil(0);

    /*glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/

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

GLvoid GLEngine::buildTextureFont(GLvoid)
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
	//24 min VTM 23 Drawing text???
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

//This WORKS
/*void GLEngine::drawModelUsingProgrammablePipeline()
{
	const ModelOBJ::Mesh *pMesh = 0;
    const ModelOBJ::Material *pMaterial = 0;
    const ModelOBJ::Vertex *pVertices = 0;
    ModelTextures::const_iterator iter;
    GLuint texture = 0;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < model.getNumberOfMeshes(); ++i)
    {
        pMesh = &model.getMesh(i);
        pMaterial = pMesh->pMaterial;
        pVertices = model.getVertexBuffer();

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pMaterial->ambient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pMaterial->diffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pMaterial->specular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pMaterial->shininess * 128.0f);

        if (pMaterial->bumpMapFilename.empty())
        {
             //Per fragment Blinn-Phong code path.

            glUseProgram(blinnPhongShader);

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
        }
       
         //Render mesh.

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

        if (model.hasTangents())
        {
            glClientActiveTexture(GL_TEXTURE1);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(4, GL_FLOAT, model.getVertexSize(),
                model.getVertexBuffer()->tangent);
        }

        glDrawElements(GL_TRIANGLES, pMesh->triangleCount * 3, GL_UNSIGNED_INT,
            model.getIndexBuffer() + pMesh->startIndex);

        if (model.hasTangents())
        {
            glClientActiveTexture(GL_TEXTURE1);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }

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

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glDisable(GL_BLEND);
}*/

//This DOES NOT WORK; COMMENTED OUT METHOD ABOVE DOES WORK
void GLEngine::drawModelUsingProgrammablePipeline()
{
	ModelTextures::const_iterator iter;
    GLuint texture = 0;

	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Vertex arrays setup
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer(3, GL_FLOAT, model.getVertexSize(), model.getVertexBuffer()->position);

	glEnableClientState( GL_NORMAL_ARRAY );
	glNormalPointer(GL_FLOAT, model.getVertexSize(), model.getVertexBuffer()->normal);

	glClientActiveTexture( GL_TEXTURE0 );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer(2, GL_FLOAT, model.getVertexSize(), model.getVertexBuffer()->texCoord);
	
	glUseProgram(blinnPhongShader);

	objects = model.getObjects();
	// Loop through objects...
	for( int i=0 ; i < (int)objects.size(); i++ ) 
	{
		ModelOBJ::GroupObject *object = objects[i];

		// Loop through materials used by object...
		for( int j=0 ; j < (int)object->materials.size() ; j++ ) 
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
			}
			//glDrawElements( GL_TRIANGLES, pMaterial->triangleCount, GL_UNSIGNED_INT, &pMaterial->indices.front() );
			glDrawElements( GL_TRIANGLES, pMaterial->triangleCount, GL_UNSIGNED_INT, model.getIndexBuffer() + pMaterial->startIndex );

		}
		
	}
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glDisable(GL_BLEND);
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
