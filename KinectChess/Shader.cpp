#include <cstdlib>
#include "Shader.h"


Shader::Shader() {}

Shader::Shader(string vertexFile, string fragFile)
{
	/*vertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	fragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	GLchar *vv, *ff;
	vv = readFile(vertexFile);
	ff = readFile(fragFile);

	if ( vv != NULL && ff != NULL )
	{
		glShaderSourceARB(vertexShader, 1, (const GLchar**)&vv, NULL);
		glShaderSourceARB(fragmentShader, 1, (const GLchar**)&ff, NULL);

		delete [] vv;
		delete [] ff;

		glCompileShaderARB(vertexShader);
		glCompileShaderARB(fragmentShader);

		program = glCreateProgramObjectARB();
		glAttachObjectARB(program, vertexShader);
		glAttachObjectARB(program, fragmentShader);

		glLinkProgramARB(program);
	}*/


}

GLuint Shader::compileShader(GLenum type, const GLchar *pszSource, GLint length)
{
    // Compiles the shader given it's source code. Returns the shader object.
    // A std::string object containing the shader's info log is thrown if the
    // shader failed to compile.
    //
    // 'type' is either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
    // 'pszSource' is a C style string containing the shader's source code.
    // 'length' is the length of 'pszSource'.

    GLuint shader = glCreateShader(type);

    if (shader)
    {
        GLint compiled = 0;

        glShaderSource(shader, 1, &pszSource, &length);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if (!compiled)
        {
            GLsizei infoLogSize = 0;
            std::string infoLog;

            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogSize);
            infoLog.resize(infoLogSize);
            glGetShaderInfoLog(shader, infoLogSize, &infoLogSize, &infoLog[0]);

            throw infoLog;
        }
    }

    return shader;
}

GLuint Shader::linkShaders(GLuint vertShader, GLuint fragShader)
{
    // Links the compiled vertex and/or fragment shaders into an executable
    // shader program. Returns the executable shader object. If the shaders
    // failed to link into an executable shader program, then a std::string
    // object is thrown containing the info log.

    GLuint program = glCreateProgram();

    if (program)
    {
        GLint linked = 0;

        if (vertShader)
            glAttachShader(program, vertShader);

        if (fragShader)
            glAttachShader(program, fragShader);

        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &linked);

        if (!linked)
        {
            GLsizei infoLogSize = 0;
            std::string infoLog;

            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogSize);
            infoLog.resize(infoLogSize);
            glGetProgramInfoLog(program, infoLogSize, &infoLogSize, &infoLog[0]);

            throw infoLog;
        }

        // Mark the two attached shaders for deletion. These two shaders aren't
        // deleted right now because both are already attached to a shader
        // program. When the shader program is deleted these two shaders will
        // be automatically detached and deleted.

        if (vertShader)
            glDeleteShader(vertShader);

        if (fragShader)
            glDeleteShader(fragShader);
    }

    return program;
}

void Shader::readTextFileFromResource(const char *pResouceId, std::string &buffer)
{
    HMODULE hModule = GetModuleHandle(0);
    HRSRC hResource = FindResource(hModule, pResouceId, RT_RCDATA);

    if (hResource)
    {
        DWORD dwSize = SizeofResource(hModule, hResource);
        HGLOBAL hGlobal = LoadResource(hModule, hResource);

        if (hGlobal)
        {
            if (LPVOID pData = LockResource(hGlobal))
            {
                buffer.assign(reinterpret_cast<const char *>(pData), dwSize);
                UnlockResource(hGlobal);
            }
        }
    }
}

GLuint Shader::loadShaderProgramFromFile(const char *fn, std::string &infoLog)
{
    infoLog.clear();

    GLuint program = 0;
    std::string buffer;

	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {
		fp = fopen(fn,"rt");

		if (fp != NULL) {

			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	buffer = (std::string)content;
	if ( buffer.length() == 0 )
        return program;

    program = loadShaderProgram( buffer, infoLog );
    //if ( buffer.length() == 0 && *pResourceId != NULL )
    //buffer = readFile( pResourceId );

    return program;
}

GLuint Shader::loadShaderProgramFromResource(const char *pResourceId, std::string &infoLog)
{
    infoLog.clear();

    GLuint program = 0;
    std::string buffer;

    // Read the text file containing the GLSL shader program.
    // This file contains 1 vertex shader and 1 fragment shader.
    readTextFileFromResource(pResourceId, buffer);

    if ( buffer.length() == 0 )
        return program;

    program = loadShaderProgram( buffer, infoLog );
    //if ( buffer.length() == 0 && *pResourceId != NULL )
    //buffer = readFile( pResourceId );

    return program;
}

GLuint Shader::loadShaderProgram(std::string buffer, std::string &infoLog)
{
    infoLog.clear();

    GLuint program = 0;
    //std::string buffer;

    // Read the text file containing the GLSL shader program.
    // This file contains 1 vertex shader and 1 fragment shader.
    // Compile and link the vertex and fragment shaders.
    if (buffer.length() > 0)
    {
        const GLchar *pSource = 0;
        GLint length = 0;
        GLuint vertShader = 0;
        GLuint fragShader = 0;

        std::string::size_type vertOffset = buffer.find("[vert]");
        std::string::size_type fragOffset = buffer.find("[frag]");

        try
        {
            // Get the vertex shader source and compile it.
            // The source is between the [vert] and [frag] tags.
            if (vertOffset != std::string::npos)
            {
                vertOffset += 6;        // skip over the [vert] tag
                pSource = reinterpret_cast<const GLchar *>(&buffer[vertOffset]);
                length = static_cast<GLint>(fragOffset - vertOffset);
                vertShader = Shader::compileShader(GL_VERTEX_SHADER, pSource, length);
            }

            // Get the fragment shader source and compile it.
            // The source is between the [frag] tag and the end of the file.
            if (fragOffset != std::string::npos)
            {
                fragOffset += 6;        // skip over the [frag] tag
                pSource = reinterpret_cast<const GLchar *>(&buffer[fragOffset]);
                length = static_cast<GLint>(buffer.length() - fragOffset - 1);
                fragShader = Shader::compileShader(GL_FRAGMENT_SHADER, pSource, length);
            }

            // Now link the vertex and fragment shaders into a shader program.
            program = Shader::linkShaders(vertShader, fragShader);
        }
        catch (const std::string &errors)
        {
            infoLog = errors;
        }
    }

    return program;
}

void Shader::useShader(void)
{
	glUseProgramObjectARB(program);
}

void Shader::setAttribute1i(string name, int x)
{
	glUniform1iARB( glGetUniformLocationARB(program, name.data()), x );
}

void Shader::setAttribute1f(string name, float x)
{
	glUniform1fARB( glGetUniformLocationARB(program, name.data()), x );
}

void Shader::setAttribute2f(string name, float x, float y)
{
	glUniform2fARB( glGetUniformLocationARB(program, name.data()), x, y );
}

void Shader::setAttribute3f(string name, float x, float y, float z)
{
	glUniform3fARB( glGetUniformLocationARB(program, name.data()), x, y, z );
}

void Shader::setAttribute4f(string name, float x, float y, float z, float w)
{
	glUniform4fARB( glGetUniformLocationARB(program, name.data()), x, y, z, w );
}

void Shader::disableShaders(void)
{
	glUseProgramObjectARB(NULL);
}


//Debugging shaders
//int compiled = 0, length = 0, laux = 0;
//GLcharARB *logString;

//Debugging shaders
//glGetObjectParameterivARB(p, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
//glGetObjectParameterivARB(p, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
//logString = (GLcharARB *)malloc(length * sizeof(GLcharARB));
//glGetInfoLogARB(p, length, &laux, logString);
