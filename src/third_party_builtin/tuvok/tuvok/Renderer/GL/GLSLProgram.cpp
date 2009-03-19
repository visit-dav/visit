/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/**
  \file    GLSLProgram.cpp
  \author  Jens Schneider, Jens Krueger
           SCI Institute, University of Utah
  \date    October 2008
*/

#ifdef _WIN32
  #pragma warning( disable : 4996 ) // disable deprecated warning
#endif

#include "GLSLProgram.h"
#include <Controller/Controller.h>

bool GLSLProgram::m_bGlewInitialized=true;    ///< GL Extension Wrangler (glew) is initialized on first instantiation
bool GLSLProgram::m_bGLChecked=false;         ///< GL extension check
bool GLSLProgram::m_bGLUseARB=false;          ///< use pre GL 2.0 syntax

/*
Hack: since the ATI/AMD driver has a bug and the 3rd parameter if glGetActiveUniform must be != 0 we use these two dummy vars in the glGetActiveUniform calls
*/
GLsizei AtiHackLen;
GLchar AtiHackChar;

/**
 * Default Constructor.
 * Initializes glew on first instantiation.
 * \param void
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 * \see Initialize()
 */
GLSLProgram::GLSLProgram(MasterController* pMasterController) :
  m_pMasterController(pMasterController),
  m_bInitialized(false),
  m_bEnabled(false),
  m_hProgram(0)
{
    Initialize();
}

/**
 * Specialized Constructor.
 * Loads any combination of vertex and fragment shader from disk.
 * \param VSFile - name of the file containing the vertex shader
 * \param FSFile - name of the file containing the fragment shader
 * \param src - selects the source of vertex and fragment shader. Can be either GLSLPROGRAM_DISK or GLSLPROGRAM_STRING
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 * \see Initialize()
 * \see Load()
 * \see GLSLPROGRAM_SOURCE
 */
GLSLProgram::GLSLProgram(MasterController* pMasterController, const char *VSFile, const char *FSFile,GLSLPROGRAM_SOURCE src) :
  m_pMasterController(pMasterController),
  m_bInitialized(false),
  m_bEnabled(false),
  m_hProgram(0)
{
  if (Initialize())
    Load(VSFile,FSFile,src);
}



/**
 * Standard Destructor.
 * Cleans up the memory automatically.
 * \param void
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 */
GLSLProgram::~GLSLProgram() {
  if (IsValid()) {
    if (m_bGLUseARB)
      glDeleteObjectARB(m_hProgram);
    else
      glDeleteProgram(m_hProgram);
  }
  m_hProgram=0;
}



/**
 * Returns the handle to this shader.
 * \param void
 * \return a const handle. If this is an invalid shader, the handle will be 0.
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Feb.2005
 */
GLSLProgram::operator GLuint(void) const {
  return m_hProgram;
}

/**
 * Initializes the class.
 * If GLSLProgram is initialized for the first time, initialize GLEW
 * \param void
 * \return bool
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 * \see m_bGlewInitialized
 */
bool GLSLProgram::Initialize(void) {
  if (!m_bGlewInitialized) {
    if (GLEW_OK!=glewInit()) T_ERROR("GLEW initialization failed!");
    m_bGlewInitialized=true;
  }
#ifdef GLSL_DEBUG  // just in case someone wants to handle GLEW himself (by setting the static var to true) but failed to do so properly
  else {
    if (glMultiTexCoord2f==NULL) T_ERROR("GLEW must be initialized. Set GLSLProgram::m_bGlewInitialized = false in GLSLProgram.cpp if you want this class to do it for you");
  }
#endif

  if (!m_bGLChecked) {
    if (atof((const char*)glGetString(GL_VERSION)) >= 2.0) {
      MESSAGE("OpenGL 2.0 supported");
      m_bGLUseARB = false;
    } else { // check for ARB extensions
      if (glewGetExtension("GL_ARB_shader_objects"))
        MESSAGE("ARB_shader_objects supported.");
      else {
        T_ERROR("Neither OpenGL 2.0 nor ARB_shader_objects not supported!");
        return false;
      }
      if (glewGetExtension("GL_ARB_shading_language_100"))
        MESSAGE("ARB_shading_language_100 supported.");
      else {
        MESSAGE("Neither OpenGL 2.0 nor ARB_shading_language_100 not supported!");
        return false;
      }

      glUniform1i  = glUniform1iARB;    glUniform2i  = glUniform2iARB;
      glUniform1iv = glUniform1ivARB;   glUniform2iv = glUniform2ivARB;
      glUniform3i  = glUniform3iARB;    glUniform4i  = glUniform4iARB;
      glUniform3iv = glUniform3ivARB;   glUniform4iv = glUniform4ivARB;

      glUniform1f  = glUniform1fARB;    glUniform2f  = glUniform2fARB;
      glUniform1fv = glUniform1fvARB;   glUniform2fv = glUniform2fvARB;
      glUniform3f  = glUniform3fARB;    glUniform4f  = glUniform4fARB;
      glUniform3fv = glUniform3fvARB;   glUniform4fv = glUniform4fvARB;

      glUniformMatrix2fv = glUniformMatrix2fvARB;
      glUniformMatrix3fv = glUniformMatrix3fvARB;
      glUniformMatrix4fv = glUniformMatrix4fvARB;

      m_bGLUseARB = true;
    }
  }
  return true;
}



/**
 * Loads vertex and fragment shader from disk/memory.
 * Loads any combination of vertex and fragment shader from disk or from a memory position.
 * Generates error/information messages to stdout during loading.
 * If nor successful the handle of the shader will be set to 0.
 * \param VSFile - name of the file containing the vertex shader
 * \param FSFile - name of the file containing the fragment shader
 * \param src - selects the source of vertex and fragment shader. Can be either GLSLPROGRAM_DISK or GLSLPROGRAM_STRING
 * \return void
 * \warning Uses glGetError()
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 * \see GLSLPROGRAM_SOURCE
 */
void GLSLProgram::Load(const char *VSFile, const char *FSFile, GLSLPROGRAM_SOURCE src) {
  CheckGLError();

  // load
  GLuint hVS=0;
  GLuint hFS=0;
  bool bVSSuccess=true;  // fixed function pipeline is always working
  if (VSFile!=NULL) {
    hVS=LoadShader(VSFile,GL_VERTEX_SHADER,src);
    if (hVS!=0) MESSAGE("VERTEX SHADER: OK");
    else {
      bVSSuccess=false;
      if (src==GLSLPROGRAM_DISK) {
        T_ERROR("ERROR IN: %s", VSFile);
      }
      else {
        T_ERROR("---------- ERROR -----------");
        int iPos=0;
        int iLine=1;
        char chLine[32];
        char *chVerbose=new char[strlen(VSFile)+1];
        memcpy(chVerbose,VSFile,strlen(VSFile)+1);
        for (unsigned int i=0; i<strlen(VSFile); i++) {
          if (chVerbose[i]=='\n') {
            chVerbose[i]='\0';
            sprintf(chLine,"(%.4i) ",iLine++);
            T_ERROR( "Load %s %s",chLine,&chVerbose[iPos]);
            iPos=i+1;
          }
        }
        delete[] chVerbose;
      }
    }
  }
  bool bFSSuccess=true;  // fixed function pipeline is always working
  if (FSFile!=NULL) {
    hFS=LoadShader(FSFile,GL_FRAGMENT_SHADER,src);
    if (hFS!=0) MESSAGE("FRAGMENT SHADER: OK");
    else {
      bFSSuccess=false;
      if (src==GLSLPROGRAM_DISK) {
        T_ERROR( "ERROR IN: %s",FSFile);
      }
      else {
        T_ERROR("---------- ERROR -----------");
        int iPos=0;
        int iLine=1;
        char chLine[32];
        char *chVerbose=new char[strlen(FSFile)+1];
        memcpy(chVerbose,FSFile,strlen(FSFile)+1);
        for (unsigned int i=0; i<strlen(FSFile); i++) {
          if (chVerbose[i]=='\n') {
            chVerbose[i]='\0';
            sprintf(chLine,"(%.4i) ",iLine++);
            T_ERROR( "Load %s %s",chLine, &chVerbose[iPos]);
            iPos=i+1;
          }
        }
        delete[] chVerbose;
      }
    }
  }

  if (m_bGLUseARB) {
    // attach to shader program
    m_hProgram=glCreateProgramObjectARB();
    if (hVS) glAttachObjectARB(m_hProgram,hVS);
    if (hFS) glAttachObjectARB(m_hProgram,hFS);

    // link the program together
    if (bVSSuccess && bFSSuccess) {
      glLinkProgramARB(m_hProgram);

      // check for errors
      GLint iLinked;
      glGetObjectParameterivARB(m_hProgram,GL_OBJECT_LINK_STATUS_ARB,&iLinked);
      WriteError(m_hProgram);

      // delete temporary objects
      if (hVS) glDeleteObjectARB(hVS);
      if (hFS) glDeleteObjectARB(hFS);

      if (CheckGLError("Load()") || !iLinked) {
        glDeleteObjectARB(m_hProgram);
        m_bInitialized=false;
        return;
      } else {
        MESSAGE("PROGRAM OBJECT: OK");
        m_bInitialized=true;
      }
    } else {
      if (hVS) glDeleteObjectARB(hVS);
      if (hFS) glDeleteObjectARB(hFS);
      glDeleteObjectARB(m_hProgram);
      m_hProgram=0;
      m_bInitialized=false;
      if (!bVSSuccess && !bFSSuccess) T_ERROR("Error in vertex and fragment shaders");
      else if (!bVSSuccess) T_ERROR("Error in vertex shader");
      else if (!bFSSuccess) T_ERROR("Error in fragment shader");
    }
  } else {
    // attach to program object
    m_hProgram=glCreateProgram();
    if (hVS) glAttachShader(m_hProgram,hVS);
    if (hFS) glAttachShader(m_hProgram,hFS);

    // link the program together
    if (bVSSuccess && bFSSuccess) {
      glLinkProgram(m_hProgram);

      // check for errors
      GLint iLinked;
      glGetProgramiv(m_hProgram,GL_LINK_STATUS,&iLinked);

      std::string fileDesc = std::string("VS: ") + std::string(VSFile) + std::string("  FS:") + std::string(FSFile);
      WriteInfoLog(fileDesc.c_str(), m_hProgram, true);

      // flag shaders such that they can be deleted when they get detached
      if (hVS) glDeleteShader(hVS);
      if (hFS) glDeleteShader(hFS);
      if (CheckGLError("Load()") || iLinked!=GLint(GL_TRUE)) {
        glDeleteProgram(m_hProgram);
        m_hProgram=0;
        m_bInitialized=false;
        return;
      }
      else {
        MESSAGE("PROGRAM OBJECT: OK");
        m_bInitialized=true;
      }
    }
    else {
      if (hVS) glDeleteShader(hVS);
      if (hFS) glDeleteShader(hFS);
      glDeleteProgram(m_hProgram);
      m_hProgram=0;
      m_bInitialized=false;
      if (!bVSSuccess && !bFSSuccess) T_ERROR("Error in vertex and fragment shaders");
      else if (!bVSSuccess) T_ERROR("Error in vertex shader");
      else if (!bFSSuccess) T_ERROR("Error in fragment shader");
    }
  }
}



/**
 * Writes errors/information messages to stdout.
 * Gets the InfoLogARB of hObject and messages it.
 * \param hObject - a handle to the object.
 * \param bProgram - if true, hObject is a program object, otherwise it is a shader object.
 * \return true: InfoLogARB non-empty and GLSLPROGRAM_STRICT defined OR only warning, false otherwise
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 */
bool GLSLProgram::WriteInfoLog(const char* shaderdesc, GLuint hObject, bool bProgram) {
  // Check for errors
  GLint iLength;
  if (bProgram)
    glGetProgramiv(hObject,GL_INFO_LOG_LENGTH,&iLength);
  else
    glGetShaderiv(hObject,GL_INFO_LOG_LENGTH,&iLength);

  GLboolean bAtMostWarnings=true;
  if (iLength>1) {
    char *pcLogInfo=new char[iLength];
    if (bProgram) {
      glGetProgramInfoLog(hObject,iLength,&iLength,pcLogInfo);
      bAtMostWarnings=glIsProgram(hObject);
    }
    else {
      glGetShaderInfoLog(hObject,iLength,&iLength,pcLogInfo);
      bAtMostWarnings=glIsShader(hObject);
    }
    if (bAtMostWarnings) {
      WARNING(shaderdesc);
      WARNING(pcLogInfo);
    delete[] pcLogInfo;
    return false;
    } else {
      T_ERROR(shaderdesc);
      T_ERROR(pcLogInfo);
    delete[] pcLogInfo;
#ifdef GLSLPROGRAM_STRICT
    return true;
#endif
    }
  }
  return !bool(bAtMostWarnings==GL_TRUE); // error occured?
}

/**
 * Writes errors/information messages to stdout.
 * Gets the InfoLogARB and writes it to stdout.
 * Parameter is necessary for temporary objects (i.e. vertex / fragment shader)
 * \param hObject - a handle to the object.
 * \return true if InfoLogARB non-empty (i.e. error/info message), false otherwise
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 */
bool GLSLProgram::WriteError(GLhandleARB hObject) {
  // Check for errors
  GLint iLength;
  glGetObjectParameterivARB(hObject,GL_OBJECT_INFO_LOG_LENGTH_ARB,&iLength);
  if (iLength>1) {
    GLcharARB *pcLogInfo=new GLcharARB[iLength];
    glGetInfoLogARB(hObject,iLength,&iLength,pcLogInfo);
    MESSAGE(pcLogInfo);
    delete[] pcLogInfo;
    return true;  // an error had occured.
  }
  return false;
}



/**
 * Loads a vertex or fragment shader.
 * Loads either a vertex or fragment shader and tries to compile it.
 * \param ShaderDesc - name of the file containing the shader
 * \param Type - either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
 * \param src - defines the source of the shader. Can be either GLSLPROGRAM_DISK or GLSLPROGRAM_STRING.
 * \return a handle to the compiled shader if successful, 0 otherwise
 * \warning uses glGetError()
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Mar.2005
 * \see GLSLPROGRAM_SOURCE
 */
GLuint GLSLProgram::LoadShader(const char *ShaderDesc, GLenum Type, GLSLPROGRAM_SOURCE src) {
  // assert right type
  assert(Type==GL_VERTEX_SHADER || Type==GL_FRAGMENT_SHADER);

  CheckGLError();

  unsigned long lFileSize;
  char *pcShader;
  FILE *fptr;

  // Load and compile vertex shader
  switch(src) {
    case GLSLPROGRAM_DISK:
      fptr=fopen(ShaderDesc,"rb");
      if (!fptr) {
        T_ERROR("File %s not found!",ShaderDesc);
        return 0;
      }
      if (fseek(fptr,0,SEEK_END)) {
        fclose(fptr);
        T_ERROR("Error reading file %s.",ShaderDesc);
        return 0;
      }
      lFileSize=ftell(fptr)/sizeof(char);
      fseek(fptr,0,SEEK_SET);
      pcShader=new char[lFileSize+1];
      pcShader[lFileSize]='\0';
      if (lFileSize!=fread(pcShader,sizeof(char),lFileSize,fptr)) {
        fclose(fptr);
        delete[] pcShader;
        T_ERROR("Error reading file %s.",ShaderDesc);
        return 0;
      }
      fclose(fptr);
      break;
    case GLSLPROGRAM_STRING:
      pcShader=(char*)ShaderDesc;
      lFileSize=long(strlen(pcShader));
      break;
    default:
      T_ERROR("Unknown source");
      return 0;
      break;
  }

  GLuint hShader = 0;
  bool bError=false;
  if (m_bGLUseARB) {
    hShader = glCreateShaderObjectARB(Type);
    glShaderSourceARB(hShader,1,(const GLchar**)&pcShader,NULL); // upload null-terminated shader
    glCompileShaderARB(hShader);

    // Check for errors
    if (CheckGLError("LoadProgram()")) {
      glDeleteObjectARB(hShader);
      bError =true;
    }
  } else {
    hShader = glCreateShader(Type);
    glShaderSource(hShader,1,(const char**)&pcShader,NULL);  // upload null-terminated shader
    glCompileShader(hShader);

    // Check for compile status
    GLint iCompiled;
    glGetShaderiv(hShader,GL_COMPILE_STATUS,&iCompiled);

    // Check for errors
    if (WriteInfoLog(ShaderDesc, hShader,false)) {
      glDeleteShader(hShader);
      bError=true;
    }

    if (CheckGLError("LoadProgram()") || iCompiled!=GLint(GL_TRUE)) {
      glDeleteShader(hShader);
      bError=true;
    }
  }

  if (pcShader!=ShaderDesc) delete[] pcShader;

  if (bError) return 0;
  return hShader;
}



/**
 * Enables the program for rendering.
 * Generates error messages if something went wrong (i.e. program not initialized etc.)
 * \param void
 * \return void
 * \warning uses glGetError()
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 */
void GLSLProgram::Enable(void) {
  if (m_bInitialized) {
    CheckGLError();
    if (m_bGLUseARB)
      glUseProgramObjectARB(m_hProgram);
    else
      glUseProgram(m_hProgram);
    if (!CheckGLError("Enable()")) m_bEnabled=true;
  }
  else T_ERROR("No program loaded!");
}



/**
 * Disables the program for rendering.
 * Generates error messages if something went wrong (i.e. program not initialized etc.)
 * \param void
 * \return void
 * \warning uses glGetError()
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 */
void GLSLProgram::Disable(void) {
  if (m_bInitialized) {
    CheckGLError();
    if (m_bGLUseARB)
      glUseProgramObjectARB(0);
    else
      glUseProgram(0);
    if (!CheckGLError("Disable()")) m_bEnabled=false;
  }
  else T_ERROR("No program loaded!");
}



/**
 * Checks and handles glErrors.
 * This routine is verbose when run with GLSL_DEBUG defined, only.
 * If called with NULL as parameter, queries glGetError() and returns true if glGetError() did not return GL_NO_ERROR.
 * If called with a non-NULL parameter, queries glGetError() and concatenates pcError and the verbosed glError.
 * If in debug mode, the error is output to stderr, otherwise it is silently ignored.
 * \param pcError first part of an error message. May be NULL.
 * \param pcAdditional additional part of error message. May be NULL.
 * \return bool specifying if an error occured (true) or not (false)
 * \warning uses glGetError()
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 */
#ifndef GLSL_DEBUG
bool GLSLProgram::CheckGLError(const char*, const char*) const {
  return (glGetError()!=GL_NO_ERROR);
}
#else
bool GLSLProgram::CheckGLError(const char *pcError, const char *pcAdditional) const{
  if (pcError==NULL) {  // Simply check for error, true if an error occured.
    return (glGetError()!=GL_NO_ERROR);
  }
  else {  // print out error
    GLenum iError=glGetError();
    char *pcMessage;
    if (pcAdditional) {
      size_t len=16+strlen(pcError)+(pcAdditional ? strlen(pcAdditional) : 0);
      pcMessage=new char[len];
      sprintf(pcMessage,pcError,pcAdditional);
    }
    else pcMessage=(char*)pcError;

    char *output=new char[strlen(pcMessage)+128];
    switch (iError) {
      case GL_NO_ERROR:
        if (pcMessage!=pcError) delete[] pcMessage;
        return false;
        break;
      case GL_INVALID_ENUM:       sprintf(output,"%s - %s",pcMessage,"GL_INVALID_ENUM");    break;
      case GL_INVALID_VALUE:      sprintf(output,"%s - %s",pcMessage,"GL_INVALID_VALUE");    break;
      case GL_INVALID_OPERATION:  sprintf(output,"%s - %s",pcMessage,"GL_INVALID_OPERATION");  break;
      case GL_STACK_OVERFLOW:     sprintf(output,"%s - %s",pcMessage,"GL_STACK_OVERFLOW");  break;
      case GL_STACK_UNDERFLOW:    sprintf(output,"%s - %s",pcMessage,"GL_STACK_UNDERFLOW");  break;
      case GL_OUT_OF_MEMORY:      sprintf(output,"%s - %s",pcMessage,"GL_OUT_OF_MEMORY");    break;
      default:                    sprintf(output,"%s - unknown GL_ERROR",pcError);      break;
    }
    if (pcMessage!=pcError) delete[] pcMessage;

    // display the error.
    T_ERROR(output);
    delete[] output;

    return true;
  }
}
#endif


/**************************************************************************************************************

(c) 2004-05 by Jens Schneider, TUM.3D
  mailto:jens.schneider@in.tum.de
  Computer Graphics and Visualization Group
    Institute for Computer Science I15
  Technical University of Munich

**************************************************************************************************************/

/**
 * Returns true if this program is valid.
 * \param void
 * \return true if this program was initialized properly
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Jun.2005
 */
bool GLSLProgram::IsValid(void) const {
  return m_bInitialized;
}

/**
 * Sets an uniform vector parameter.
 * \warning uses glGetError();
 * \param name - name of the parameter
 * \param x,y,z,w - up to four float components of the vector to set.
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 */
void GLSLProgram::SetUniformVector(const char *name,float x, float y, float z, float w) const{
  assert(m_bEnabled);
  CheckGLError();

  GLint iSize;
  GLenum iType;
  GLint iLocation;
  if (m_bGLUseARB)
    iLocation=glGetUniformLocationARB(m_hProgram,name); // Position of uniform var
  else
    iLocation=glGetUniformLocation(m_hProgram,name); // Position of uniform var

  if (CheckGLError("SetUniformVector(%s,float,...) [getting adress]",name)) return;

  if(iLocation==-1) {
    T_ERROR("Error getting address for %s.",name);
    return;
  }

  if (m_bGLUseARB)
    glGetActiveUniformARB(m_hProgram,iLocation,0,NULL,&iSize,&iType,NULL);
  else
    glGetActiveUniform(m_hProgram,iLocation,1,&AtiHackLen,&iSize,&iType,&AtiHackChar);

  if (CheckGLError("SetUniformVector(%s,float,...) [getting type]",name)) return;

  switch (iType) {
    case GL_FLOAT:               glUniform1f(iLocation,x); break;
    case GL_FLOAT_VEC2:          glUniform2f(iLocation,x,y); break;
    case GL_FLOAT_VEC3:          glUniform3f(iLocation,x,y,z); break;
    case GL_FLOAT_VEC4:          glUniform4f(iLocation,x,y,z,w); break;

#ifdef GLSL_ALLOW_IMPLICIT_CASTS
    case GL_INT:
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_2D_RECT_ARB:
    case GL_SAMPLER_2D_RECT_SHADOW_ARB:  glUniform1i(iLocation,int(x)); break;

    case GL_INT_VEC2:          glUniform2i(iLocation,int(x),int(y)); break;
    case GL_INT_VEC3:          glUniform3i(iLocation,int(x),int(y),int(z)); break;
    case GL_INT_VEC4:          glUniform4i(iLocation,int(x),int(y),int(z),int(w)); break;
    case GL_BOOL:              glUniform1f(iLocation,x); break;
    case GL_BOOL_VEC2:          glUniform2f(iLocation,x,y); break;
    case GL_BOOL_VEC3:          glUniform3f(iLocation,x,y,z); break;
    case GL_BOOL_VEC4:          glUniform4f(iLocation,x,y,z,w); break;
#endif

    default:
      T_ERROR("Unknown type for %s.",name);
      break;
  }
#ifdef GLSL_DEBUG
  CheckGLError("SetUniformVector(%s,float,...)",name);
#endif
}



/**
 * Sets an uniform vector parameter.
 * \warning uses glGetError();
 * \param name - name of the parameter
 * \param x,y,z,w - up to four bool components of the vector to set.
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Mar.2005
 */
void GLSLProgram::SetUniformVector(const char *name,bool x, bool y, bool z, bool w) const {
  assert(m_bEnabled);
  CheckGLError();

  GLint iSize;
  GLenum iType;
  GLint iLocation;
  if (m_bGLUseARB)
    iLocation=glGetUniformLocationARB(m_hProgram,name); // Position of uniform var
  else
    iLocation=glGetUniformLocation(m_hProgram,name); // Position of uniform var

  if (CheckGLError("SetUniformVector(%s,bool,...) [getting adress]",name)) return;

  if(iLocation==-1) {
    T_ERROR("Error getting address for %s.",name);
    return;
  }

  if (m_bGLUseARB)
    glGetActiveUniformARB(m_hProgram,iLocation,0,NULL,&iSize,&iType,NULL);
  else
    glGetActiveUniform(m_hProgram,iLocation,1,&AtiHackLen,&iSize,&iType,&AtiHackChar);

  if (CheckGLError("SetUniformVector(%s,bool,...) [getting type]",name)) return;

  switch (iType) {
    case GL_BOOL:            glUniform1i(iLocation,(x ? 1 : 0)); break;
    case GL_BOOL_VEC2:          glUniform2i(iLocation,(x ? 1 : 0),(y ? 1 : 0)); break;
    case GL_BOOL_VEC3:          glUniform3i(iLocation,(x ? 1 : 0),(y ? 1 : 0),(z ? 1 : 0)); break;
    case GL_BOOL_VEC4:          glUniform4i(iLocation,(x ? 1 : 0),(y ? 1 : 0),(z ? 1 : 0),(w ? 1 : 0)); break;

#ifdef GLSL_ALLOW_IMPLICIT_CASTS
    case GL_FLOAT:            glUniform1f(iLocation,(x ? 1.0f : 0.0f)); break;
    case GL_FLOAT_VEC2:          glUniform2f(iLocation,(x ? 1.0f : 0.0f),(y ? 1.0f : 0.0f)); break;
    case GL_FLOAT_VEC3:          glUniform3f(iLocation,(x ? 1.0f : 0.0f),(y ? 1.0f : 0.0f),(z ? 1.0f : 0.0f)); break;
    case GL_FLOAT_VEC4:          glUniform4f(iLocation,(x ? 1.0f : 0.0f),(y ? 1.0f : 0.0f),(z ? 1.0f : 0.0f),(w ? 1.0f : 0.0f)); break;
    case GL_INT:            glUniform1i(iLocation,(x ? 1 : 0)); break;
    case GL_INT_VEC2:          glUniform2i(iLocation,(x ? 1 : 0),(y ? 1 : 0)); break;
    case GL_INT_VEC3:          glUniform3i(iLocation,(x ? 1 : 0),(y ? 1 : 0),(z ? 1 : 0)); break;
    case GL_INT_VEC4:          glUniform4i(iLocation,(x ? 1 : 0),(y ? 1 : 0),(z ? 1 : 0),(w ? 1 : 0)); break;
#endif

    default:
      T_ERROR("Unknown type for %s.",name);
      break;
  }
#ifdef GLSL_DEBUG
  CheckGLError("SetUniformVector(%s,bool,...)",name);
#endif
}



/**
 * Sets an uniform vector parameter.
 * \warning uses glGetError();
 * \param name - name of the parameter
 * \param x,y,z,w - four int components of the vector to set.
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 */
void GLSLProgram::SetUniformVector(const char *name,int x,int y,int z,int w) const {
  assert(m_bEnabled);
  CheckGLError();

  GLint iSize;
  GLenum iType;
  GLint iLocation;
  if (m_bGLUseARB)
    iLocation=glGetUniformLocationARB(m_hProgram,name); // Position of uniform var
  else
    iLocation=glGetUniformLocation(m_hProgram,name); // Position of uniform var

  if (CheckGLError("SetUniformVector(%s,int,...) [getting adress]", name )) return;
  if(iLocation==-1) {
    T_ERROR("Error getting address for %s.",name);
    return;
  }


  if (m_bGLUseARB)
    glGetActiveUniformARB(m_hProgram,iLocation,0,NULL,&iSize,&iType,NULL);
  else
    glGetActiveUniform(m_hProgram,iLocation,1,&AtiHackLen,&iSize,&iType,&AtiHackChar);

  if (CheckGLError("SetUniformVector(%s,int,...) [getting type]",name)) return;

  switch (iType) {
    case GL_INT:
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_2D_RECT_ARB:
    case GL_SAMPLER_2D_RECT_SHADOW_ARB:  glUniform1i(iLocation,x); break;

    case GL_INT_VEC2:          glUniform2i(iLocation,x,y); break;
    case GL_INT_VEC3:          glUniform3i(iLocation,x,y,z); break;
    case GL_INT_VEC4:          glUniform4i(iLocation,x,y,z,w); break;

#ifdef GLSL_ALLOW_IMPLICIT_CASTS
    case GL_BOOL:            glUniform1i(iLocation,x); break;
    case GL_BOOL_VEC2:          glUniform2i(iLocation,x,y); break;
    case GL_BOOL_VEC3:          glUniform3i(iLocation,x,y,z); break;
    case GL_BOOL_VEC4:          glUniform4i(iLocation,x,y,z,w); break;
    case GL_FLOAT:            glUniform1f(iLocation,float(x)); break;
    case GL_FLOAT_VEC2:          glUniform2f(iLocation,float(x),float(y)); break;
    case GL_FLOAT_VEC3:          glUniform3f(iLocation,float(x),float(y),float(z)); break;
    case GL_FLOAT_VEC4:          glUniform4f(iLocation,float(x),float(y),float(z),float(w)); break;
#endif

    default:
      T_ERROR("Unknown type for %s.",name);
      break;
  }
#ifdef GLSL_DEBUG
  CheckGLError("SetUniformVector(%s,int,...)",name);
#endif
}



/**
 * Sets an uniform vector parameter.
 * \warning uses glGetError();
 * \param name - name of the parameter
 * \param v - a float vector containing up to four elements.
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 */
void GLSLProgram::SetUniformVector(const char *name,const float *v) const {
  assert(m_bEnabled);
  CheckGLError();

  GLint iSize;
  GLenum iType;
  GLint iLocation;
  if (m_bGLUseARB)
    iLocation=glGetUniformLocationARB(m_hProgram,name); // Position of uniform var
  else
    iLocation=glGetUniformLocation(m_hProgram,name); // Position of uniform var

  if (CheckGLError("SetUniformVector(%s,float*) [getting adress]",name)) return;

  if(iLocation==-1) {
    T_ERROR("Error getting address for %s.",name);
    return;
  }

  if (m_bGLUseARB)
    glGetActiveUniformARB(m_hProgram,iLocation,0,NULL,&iSize,&iType,NULL);
  else
    glGetActiveUniform(m_hProgram,iLocation,1,&AtiHackLen,&iSize,&iType,&AtiHackChar);

  if (CheckGLError("SetUniformVector(%s,float*) [getting type]" ,name)) return;

  switch (iType) {
    case GL_FLOAT:            glUniform1fv(iLocation,1,v); break;
    case GL_FLOAT_VEC2:          glUniform2fv(iLocation,1,v); break;
    case GL_FLOAT_VEC3:          glUniform3fv(iLocation,1,v); break;
    case GL_FLOAT_VEC4:          glUniform4fv(iLocation,1,v); break;

#ifdef GLSL_ALLOW_IMPLICIT_CASTS
    case GL_INT:
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_2D_RECT_ARB:
    case GL_SAMPLER_2D_RECT_SHADOW_ARB:  glUniform1i(iLocation,int(v[0])); break;

    case GL_INT_VEC2:          glUniform2i(iLocation,int(v[0]),int(v[1])); break;
    case GL_INT_VEC3:          glUniform3i(iLocation,int(v[0]),int(v[1]),int(v[2])); break;
    case GL_INT_VEC4:          glUniform4i(iLocation,int(v[0]),int(v[1]),int(v[2]),int(v[3])); break;
    case GL_BOOL:            glUniform1fv(iLocation,1,v); break;
    case GL_BOOL_VEC2:          glUniform2fv(iLocation,1,v); break;
    case GL_BOOL_VEC3:          glUniform3fv(iLocation,1,v); break;
    case GL_BOOL_VEC4:          glUniform4fv(iLocation,1,v); break;
#endif

    default:
      T_ERROR("Unknown type for %s.",name);
      break;
  }
#ifdef GLSL_DEBUG
  CheckGLError("SetUniformVector(%s,float*)",name);
#endif
}



/**
 * Sets an uniform vector parameter.
 * \warning uses glGetError();
 * \param name - name of the parameter
 * \param i - an int vector containing up to 4 elements.
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Aug.2004
 */
void GLSLProgram::SetUniformVector(const char *name,const int *i) const {
  assert(m_bEnabled);
  CheckGLError();

  GLint iSize;
  GLenum iType;
  GLint iLocation;
  if (m_bGLUseARB)
    iLocation=glGetUniformLocationARB(m_hProgram,name); // Position of uniform var
  else
    iLocation=glGetUniformLocation(m_hProgram,name); // Position of uniform var

  if (CheckGLError("SetUniformVector(%s,int*) [getting adress]",name)) return;
  if(iLocation==-1) {
    T_ERROR("Error getting address for %s.",name);
    return;
  }

  if (m_bGLUseARB)
    glGetActiveUniformARB(m_hProgram,iLocation,0,NULL,&iSize,&iType,NULL);
  else
    glGetActiveUniform(m_hProgram,iLocation,1,&AtiHackLen,&iSize,&iType,&AtiHackChar);

  if (CheckGLError("SetUniformVector(%s,int*) [getting type]",name)) return;

  switch (iType) {
    case GL_INT:
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_2D_RECT_ARB:
    case GL_SAMPLER_2D_RECT_SHADOW_ARB:  glUniform1i(iLocation,i[0]); break;

    case GL_INT_VEC2:          glUniform2iv(iLocation,1,(const GLint*)i); break;
    case GL_INT_VEC3:          glUniform3iv(iLocation,1,(const GLint*)i); break;
    case GL_INT_VEC4:          glUniform4iv(iLocation,1,(const GLint*)i); break;
#ifdef GLSL_ALLOW_IMPLICIT_CASTS
    case GL_BOOL:            glUniform1iv(iLocation,1,(const GLint*)i); break;
    case GL_BOOL_VEC2:          glUniform2iv(iLocation,1,(const GLint*)i); break;
    case GL_BOOL_VEC3:          glUniform3iv(iLocation,1,(const GLint*)i); break;
    case GL_BOOL_VEC4:          glUniform4iv(iLocation,1,(const GLint*)i); break;
    case GL_FLOAT:            glUniform1f(iLocation,float(i[0])); break;
    case GL_FLOAT_VEC2:          glUniform2f(iLocation,float(i[0]),float(i[1])); break;
    case GL_FLOAT_VEC3:          glUniform3f(iLocation,float(i[0]),float(i[1]),float(i[2])); break;
    case GL_FLOAT_VEC4:          glUniform4f(iLocation,float(i[0]),float(i[1]),float(i[2]),float(i[3])); break;
#endif
    default:
      T_ERROR("Unknown type for %s.",name);
      break;
  }
#ifdef GLSL_DEBUG
  CheckGLError("SetUniformVector(%s,int*)",name);
#endif
}



/**
 * Sets an uniform vector parameter.
 * \warning uses glGetError();
 * \param name - name of the parameter
 * \param b - a bool vector containing up to 4 elements.
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Mar.2005
 */
void GLSLProgram::SetUniformVector(const char *name,const bool *b) const {
  assert(m_bEnabled);
  CheckGLError();

  GLint iSize;
  GLenum iType;
  GLint iLocation;
  if (m_bGLUseARB)
    iLocation=glGetUniformLocationARB(m_hProgram,name); // Position of uniform var
  else
    iLocation=glGetUniformLocation(m_hProgram,name); // Position of uniform var

  if (CheckGLError("SetUniformVector(%s,bool*) [getting adress]",name)) return;
  if(iLocation==-1) {
    T_ERROR("Error getting address for %s.",name);
    return;
  }

  if (m_bGLUseARB)
    glGetActiveUniformARB(m_hProgram,iLocation,0,NULL,&iSize,&iType,NULL);
  else
    glGetActiveUniform(m_hProgram,iLocation,1,&AtiHackLen,&iSize,&iType,&AtiHackChar);

  if (CheckGLError("SetUniformVector(%s,bool*) [getting type]",name)) return;

  switch (iType) {
    case GL_BOOL:            glUniform1i(iLocation,(b[0] ? 1 : 0)); break;
    case GL_BOOL_VEC2:          glUniform2i(iLocation,(b[0] ? 1 : 0),(b[1] ? 1 : 0)); break;
    case GL_BOOL_VEC3:          glUniform3i(iLocation,(b[0] ? 1 : 0),(b[1] ? 1 : 0),(b[2] ? 1 : 0)); break;
    case GL_BOOL_VEC4:          glUniform4i(iLocation,(b[0] ? 1 : 0),(b[1] ? 1 : 0),(b[2] ? 1 : 0),(b[3] ? 1 : 0)); break;
#ifdef GLSL_ALLOW_IMPLICIT_CASTS
    case GL_INT:            glUniform1i(iLocation,(b[0] ? 1 : 0)); break;
    case GL_INT_VEC2:          glUniform2i(iLocation,(b[0] ? 1 : 0),(b[1] ? 1 : 0)); break;
    case GL_INT_VEC3:          glUniform3i(iLocation,(b[0] ? 1 : 0),(b[1] ? 1 : 0),(b[2] ? 1 : 0)); break;
    case GL_INT_VEC4:          glUniform4i(iLocation,(b[0] ? 1 : 0),(b[1] ? 1 : 0),(b[2] ? 1 : 0),(b[3] ? 1 : 0)); break;
    case GL_FLOAT:            glUniform1f(iLocation,(b[0] ? 1.0f : 0.0f)); break;
    case GL_FLOAT_VEC2:          glUniform2f(iLocation,(b[0] ? 1.0f : 0.0f),(b[1] ? 1.0f : 0.0f)); break;
    case GL_FLOAT_VEC3:          glUniform3f(iLocation,(b[0] ? 1.0f : 0.0f),(b[1] ? 1.0f : 0.0f),(b[2] ? 1.0f : 0.0f)); break;
    case GL_FLOAT_VEC4:          glUniform4f(iLocation,(b[0] ? 1.0f : 0.0f),(b[1] ? 1.0f : 0.0f),(b[2] ? 1.0f : 0.0f),(b[3] ? 1.0f : 0.0f)); break;
#endif
    default:
      T_ERROR("Unknown type for %s.",name);
      break;
  }
#ifdef GLSL_DEBUG
  CheckGLError("SetUniformVector(%s,bool*)",name);
#endif
}



/**
 * Sets an uniform matrix.
 * Matrices are always of type float.
 * \warning uses glGetError();
 * \param name - name of the parameter
 * \param m - a float array containing up to 16 floats for the matrix.
 * \param bTranspose - if true, the matrix will be transposed before uploading.
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Mar.2005
 */
void GLSLProgram::SetUniformMatrix(const char *name,const float *m,bool bTranspose) const {
  assert(m_bEnabled);
  CheckGLError();

  GLint iSize;
  GLenum iType;
  GLint iLocation;
  if (m_bGLUseARB)
    iLocation=glGetUniformLocationARB(m_hProgram,name); // Position of uniform var
  else
    iLocation=glGetUniformLocation(m_hProgram,name); // Position of uniform var

  if (CheckGLError("SetUniformMatrix(%s,float*,bool) [getting adress]",name)) return;
  if(iLocation==-1) {
    T_ERROR("Error getting address for %s.",name);
    return;
  }

  if (m_bGLUseARB)
    glGetActiveUniformARB(m_hProgram,iLocation,0,NULL,&iSize,&iType,NULL);
  else
    glGetActiveUniform(m_hProgram,iLocation,1,&AtiHackLen,&iSize,&iType,&AtiHackChar);

  if (CheckGLError("SetUniformMatrix(%s,float*,bool) [getting type]",name)) return;

  switch (iType) {
    case GL_FLOAT_MAT2:          glUniformMatrix2fv(iLocation,1,bTranspose,m); break;
    case GL_FLOAT_MAT3:          glUniformMatrix3fv(iLocation,1,bTranspose,m); break;
    case GL_FLOAT_MAT4:          glUniformMatrix4fv(iLocation,1,bTranspose,m); break;
    default:
      T_ERROR("Unknown type for %s.",name);
      break;
  }
#ifdef GLSL_DEBUG
  CheckGLError("SetUniformMatrix(%s,float*,bool)",name);
#endif
}



#ifdef GLSL_ALLOW_IMPLICIT_CASTS

/**
 * Sets an uniform matrix.
 * Matrices are always of type float.
 * \warning uses glGetError();
 * \remark only available if GLSL_ALLOW_IMPLICIT_CASTS is defined.
 * \param name - name of the parameter
 * \param m - an int array containing up to 16 ints for the matrix. Ints are converted to float before uploading.
 * \param bTranspose - if true, the matrix will be transposed before uploading.
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Mar.2005
 */
void GLSLProgram::SetUniformMatrix(const char *name,const int *m, bool bTranspose) const {
  assert(m_bEnabled);
  CheckGLError();

  GLint iSize;
  GLenum iType;
  GLint iLocation;
  if (m_bGLUseARB)
    iLocation=glGetUniformLocationARB(m_hProgram,name); // Position of uniform var
  else
    iLocation=glGetUniformLocation(m_hProgram,name); // Position of uniform var

  if (CheckGLError("SetUniformMatrix(%s,int*,bool) [getting adress]",name)) return;
  if(iLocation==-1) {
    T_ERROR("Error getting address for %s.",name);
    return;
  }

  if (m_bGLUseARB)
    glGetActiveUniformARB(m_hProgram,iLocation,0,NULL,&iSize,&iType,NULL);
  else
    glGetActiveUniform(m_hProgram,iLocation,1,&AtiHackLen,&iSize,&iType,&AtiHackChar);

  if (CheckGLError("SetUniformMatrix(%s,float*,bool) [getting type]",name)) return;

  float M[16];
  switch (iType) {
    case GL_FLOAT_MAT2:
      for (unsigned int ui=0; ui<4; ui++) M[ui]=float(m[ui]);
      glUniformMatrix2fv(iLocation,1,bTranspose,M);
      break;
    case GL_FLOAT_MAT3:
      for (unsigned int ui=0; ui<9; ui++) M[ui]=float(m[ui]);
      glUniformMatrix3fv(iLocation,1,bTranspose,M);
      break;
    case GL_FLOAT_MAT4:
      for (unsigned int ui=0; ui<16; ui++) M[ui]=float(m[ui]);
      glUniformMatrix4fv(iLocation,1,bTranspose,M);
      break;
    default:
      T_ERROR("Unknown type for %s.",name);
      break;
  }
#ifdef GLSL_DEBUG
  CheckGLError("SetUniformMatrix(%s,int*,bool)",name);
#endif
}



/**
 * Sets an uniform matrix.
 * Matrices are always of type float.
 * \warning uses glGetError();
 * \remark only available if GLSL_ALLOW_IMPLICIT_CASTS is defined.
 * \param name - name of the parameter
 * \param m - an int array containing up to 16 ints for the matrix. Ints are converted to float before uploading.
 * \param bTranspose - if true, the matrix will be transposed before uploading.
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Mar.2005
 */
void GLSLProgram::SetUniformMatrix(const char *name,const bool *m, bool bTranspose) const {
  assert(m_bEnabled);
  CheckGLError();

  GLint iSize;
  GLenum iType;
  GLint iLocation;
  if (m_bGLUseARB)
    iLocation=glGetUniformLocationARB(m_hProgram,name); // Position of uniform var
  else
    iLocation=glGetUniformLocation(m_hProgram,name); // Position of uniform var

  if (CheckGLError("SetUniformMatrix(%s,int*,bool) [getting adress]",name)) return;
  if(iLocation==-1) {
    T_ERROR("Error getting address for %s.",name);
    return;
  }

  if (m_bGLUseARB)
    glGetActiveUniformARB(m_hProgram,iLocation,0,NULL,&iSize,&iType,NULL);
  else
    glGetActiveUniform(m_hProgram,iLocation,1,&AtiHackLen,&iSize,&iType,&AtiHackChar);

  if (CheckGLError("SetUniformMatrix(%s,float*,bool) [getting type]",name)) return;

  float M[16];
  switch (iType) {
    case GL_FLOAT_MAT2:
      for (unsigned int ui=0; ui<4; ui++) M[ui]=(m[ui] ? 1.0f : 0.0f);
      glUniformMatrix2fv(iLocation,1,bTranspose,M);
      break;
    case GL_FLOAT_MAT3:
      for (unsigned int ui=0; ui<9; ui++) M[ui]=(m[ui] ? 1.0f : 0.0f);
      glUniformMatrix3fv(iLocation,1,bTranspose,M);
      break;
    case GL_FLOAT_MAT4:
      for (unsigned int ui=0; ui<16; ui++) M[ui]=(m[ui] ? 1.0f : 0.0f);
      glUniformMatrix4fv(iLocation,1,bTranspose,M);
      break;
    default:
      T_ERROR("Unknown type for %s.",name);
      break;
  }
#ifdef GLSL_DEBUG
  CheckGLError("SetUniformMatrix(%s,int*,bool)",name);
#endif
}

#endif // GLSL_ALLOW_IMPLICIT_CASTS



/**
 * Sets an uniform array.
 * Sets the entire array at once. Single positions can still be set using the other SetUniform*() methods.
 * \warning uses glGetError();
 * \param name - name of the parameter
 * \param a - a float array containing enough floats to fill the entire uniform array.
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Mar.2005
 */
void GLSLProgram::SetUniformArray(const char *name,const float *a) const {
  assert(m_bEnabled);
  CheckGLError();

  GLint iSize;
  GLenum iType;
  GLint iLocation;
  if (m_bGLUseARB)
    iLocation=glGetUniformLocationARB(m_hProgram,name); // Position of uniform var
  else
    iLocation=glGetUniformLocation(m_hProgram,name); // Position of uniform var

  if (CheckGLError("SetUniformArray(%s,float*) [getting adress]",name)) return;
  if(iLocation==-1) {
    T_ERROR("Error getting address for %s.",name);
    return;
  }

  if (m_bGLUseARB)
    glGetActiveUniformARB(m_hProgram,iLocation,0,NULL,&iSize,&iType,NULL);
  else
    glGetActiveUniform(m_hProgram,iLocation,1,&AtiHackLen,&iSize,&iType,&AtiHackChar);

  if (CheckGLError("SetUniformArray(%s,float*) [getting type]",name)) return;

#ifdef GLSL_ALLOW_IMPLICIT_CASTS
  GLint *iArray;
#endif

  switch (iType) {
    case GL_FLOAT:            glUniform1fv(iLocation,iSize,a); break;
    case GL_FLOAT_VEC2:          glUniform2fv(iLocation,iSize,a); break;
    case GL_FLOAT_VEC3:          glUniform3fv(iLocation,iSize,a); break;
    case GL_FLOAT_VEC4:          glUniform4fv(iLocation,iSize,a); break;

#ifdef GLSL_ALLOW_IMPLICIT_CASTS
    case GL_BOOL:            glUniform1fv(iLocation,iSize,a); break;
    case GL_BOOL_VEC2:          glUniform2fv(iLocation,iSize,a); break;
    case GL_BOOL_VEC3:          glUniform3fv(iLocation,iSize,a); break;
    case GL_BOOL_VEC4:          glUniform4fv(iLocation,iSize,a); break;

    case GL_INT:
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_2D_RECT_ARB:
    case GL_SAMPLER_2D_RECT_SHADOW_ARB:
      iArray=new GLint[iSize];
      for (int i=0; i<iSize; i++) iArray[i]=int(a[i]);
      glUniform1iv(iLocation,iSize,iArray);
      delete[] iArray;
      break;

    case GL_INT_VEC2:
      iArray=new GLint[2*iSize];
      for (int i=0; i<2*iSize; i++) iArray[i]=int(a[i]);
      glUniform2iv(iLocation,iSize,iArray);
      delete[] iArray;
      break;
    case GL_INT_VEC3:
      iArray=new GLint[3*iSize];
      for (int i=0; i<3*iSize; i++) iArray[i]=int(a[i]);
      glUniform3iv(iLocation,iSize,iArray);
      delete[] iArray;
      break;
    case GL_INT_VEC4:
      iArray=new GLint[4*iSize];
      for (int i=0; i<4*iSize; i++) iArray[i]=int(a[i]);
      glUniform4iv(iLocation,iSize,iArray);
      delete[] iArray;
      break;
#endif

    default:
      T_ERROR("Unknown type for %s.",name);
      break;
  }
#ifdef GLSL_DEBUG
  CheckGLError("SetUniformArray(%s,float*)",name);
#endif
}



/**
 * Sets an uniform array.
 * Sets the entire array at once. Single positions can still be set using the other SetUniform*() methods.
 * \warning uses glGetError();
 * \param name - name of the parameter
 * \param a - an int array containing enough floats to fill the entire uniform array.
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Mar.2005
 */
void GLSLProgram::SetUniformArray(const char *name,const int *a) const {
  assert(m_bEnabled);
  CheckGLError();

  GLint iSize;
  GLenum iType;
  GLint iLocation;
  if (m_bGLUseARB)
    iLocation=glGetUniformLocationARB(m_hProgram,name); // Position of uniform var
  else
    iLocation=glGetUniformLocation(m_hProgram,name); // Position of uniform var

  if (CheckGLError("SetUniformArray(%s,int*) [getting adress]",name)) return;
  if(iLocation==-1) {
    T_ERROR("Error getting address for %s.",name);
    return;
  }

  if (m_bGLUseARB)
    glGetActiveUniformARB(m_hProgram,iLocation,0,NULL,&iSize,&iType,NULL);
  else
    glGetActiveUniform(m_hProgram,iLocation,1,&AtiHackLen,&iSize,&iType,&AtiHackChar);

  if (CheckGLError("SetUniformArray(%s,int*) [getting type]",name)) return;

#ifdef GLSL_ALLOW_IMPLICIT_CASTS
  float *fArray;
#endif

  switch (iType) {
    case GL_INT:
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_2D_RECT_ARB:
    case GL_SAMPLER_2D_RECT_SHADOW_ARB:  glUniform1iv(iLocation,iSize,(const GLint*)a); break;
    case GL_INT_VEC2:          glUniform2iv(iLocation,iSize,(const GLint*)a); break;
    case GL_INT_VEC3:          glUniform3iv(iLocation,iSize,(const GLint*)a); break;
    case GL_INT_VEC4:          glUniform4iv(iLocation,iSize,(const GLint*)a); break;

#ifdef GLSL_ALLOW_IMPLICIT_CASTS
    case GL_BOOL:            glUniform1iv(iLocation,iSize,(const GLint*)a); break;
    case GL_BOOL_VEC2:          glUniform2iv(iLocation,iSize,(const GLint*)a); break;
    case GL_BOOL_VEC3:          glUniform3iv(iLocation,iSize,(const GLint*)a); break;
    case GL_BOOL_VEC4:          glUniform4iv(iLocation,iSize,(const GLint*)a); break;

    case GL_FLOAT:
      fArray=new float[iSize];
      for (int i=0; i<iSize; i++) fArray[i]=float(a[i]);
      glUniform1fv(iLocation,iSize,fArray);
      delete[] fArray;
      break;
    case GL_FLOAT_VEC2:
      fArray=new float[2*iSize];
      for (int i=0; i<2*iSize; i++) fArray[i]=float(a[i]);
      glUniform2fv(iLocation,iSize,fArray);
      delete[] fArray;
      break;
    case GL_FLOAT_VEC3:
      fArray=new float[3*iSize];
      for (int i=0; i<3*iSize; i++) fArray[i]=float(a[i]);
      glUniform3fv(iLocation,iSize,fArray);
      delete[] fArray;
      break;
    case GL_FLOAT_VEC4:
      fArray=new float[4*iSize];
      for (int i=0; i<4*iSize; i++) fArray[i]=float(a[i]);
      glUniform4fv(iLocation,iSize,fArray);
      delete[] fArray;
      break;
#endif

    default:
      T_ERROR("Unknown type for %s.",name);
      break;
  }
#ifdef GLSL_DEBUG
  CheckGLError("SetUniformArray(%s,int*)",name);
#endif
}



/**
 * Sets an uniform array.
 * Sets the entire array at once. Single positions can still be set using the other SetUniform*() methods.
 * \warning uses glGetError();
 * \param name - name of the parameter
 * \param a - a bool array containing enough floats to fill the entire uniform array.
 * \return void
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date Mar.2005
 */
void GLSLProgram::SetUniformArray(const char *name,const bool  *a) const {
  assert(m_bEnabled);
  CheckGLError();

  GLint iSize;
  GLenum iType;
  GLint iLocation;
  if (m_bGLUseARB)
    iLocation=glGetUniformLocationARB(m_hProgram,name); // Position of uniform var
  else
    iLocation=glGetUniformLocation(m_hProgram,name); // Position of uniform var

  if (CheckGLError("SetUniformArray(%s,bool*) [getting adress]",name)) return;
  if(iLocation==-1) {
   T_ERROR("Error getting address for %s.",name);
    return;
  }

  if (m_bGLUseARB)
    glGetActiveUniformARB(m_hProgram,iLocation,0,NULL,&iSize,&iType,NULL);
  else
    glGetActiveUniform(m_hProgram,iLocation,1,&AtiHackLen,&iSize,&iType,&AtiHackChar);

  if (CheckGLError("SetUniformArray(%s,bool*) [getting type]",name)) return;

#ifdef GLSL_ALLOW_IMPLICIT_CASTS
  float *fArray;
#endif
  GLint *iArray;
  switch (iType) {
    case GL_BOOL:
      iArray=new GLint[iSize];
      for (int i=0; i<iSize; i++) iArray[i]=(a[i] ? 1 : 0);
      glUniform1iv(iLocation,iSize,iArray);
      delete[] iArray;
      break;
    case GL_BOOL_VEC2:
      iArray=new GLint[2*iSize];
      for (int i=0; i<2*iSize; i++) iArray[i]=(a[i] ? 1 : 0);
      glUniform2iv(iLocation,iSize,iArray);
      delete[] iArray;
      break;
    case GL_BOOL_VEC3:
      iArray=new GLint[3*iSize];
      for (int i=0; i<3*iSize; i++) iArray[i]=(a[i] ? 1 : 0);
      glUniform3iv(iLocation,iSize,iArray);
      delete[] iArray;
      break;
    case GL_BOOL_VEC4:
      iArray=new GLint[4*iSize];
      for (int i=0; i<4*iSize; i++) iArray[i]=(a[i] ? 1 : 0);
      glUniform4iv(iLocation,iSize,iArray);
      delete[] iArray;
      break;

#ifdef GLSL_ALLOW_IMPLICIT_CASTS
    case GL_INT:
      iArray=new GLint[iSize];
      for (int i=0; i<iSize; i++) iArray[i]=(a[i] ? 1 : 0);
      glUniform1iv(iLocation,iSize,iArray);
      delete[] iArray;
      break;
    case GL_INT_VEC2:
      iArray=new GLint[2*iSize];
      for (int i=0; i<2*iSize; i++) iArray[i]=(a[i] ? 1 : 0);
      glUniform2iv(iLocation,iSize,iArray);
      delete[] iArray;
      break;
    case GL_INT_VEC3:
      iArray=new GLint[3*iSize];
      for (int i=0; i<3*iSize; i++) iArray[i]=(a[i] ? 1 : 0);
      glUniform3iv(iLocation,iSize,iArray);
      delete[] iArray;
      break;
    case GL_INT_VEC4:
      iArray=new GLint[4*iSize];
      for (int i=0; i<4*iSize; i++) iArray[i]=(a[i] ? 1 : 0);
      glUniform4iv(iLocation,iSize,iArray);
      delete[] iArray;
      break;
    case GL_FLOAT:
      fArray=new float[iSize];
      for (int i=0; i<iSize; i++) fArray[i]=(a[i] ? 1.0f : 0.0f);
      glUniform1fv(iLocation,iSize,fArray);
      delete[] fArray;
      break;
    case GL_FLOAT_VEC2:
      fArray=new float[2*iSize];
      for (int i=0; i<2*iSize; i++) fArray[i]=(a[i] ? 1.0f : 0.0f);
      glUniform2fv(iLocation,iSize,fArray);
      delete[] fArray;
      break;
    case GL_FLOAT_VEC3:
      fArray=new float[3*iSize];
      for (int i=0; i<3*iSize; i++) fArray[i]=(a[i] ? 1.0f : 0.0f);
      glUniform3fv(iLocation,iSize,fArray);
      delete[] fArray;
      break;
    case GL_FLOAT_VEC4:
      fArray=new float[4*iSize];
      for (int i=0; i<4*iSize; i++) fArray[i]=(a[i] ? 1.0f : 0.0f);
      glUniform4fv(iLocation,iSize,fArray);
      delete[] fArray;
      break;
#endif

    default:
      T_ERROR("Unknown type for %s.",name);
      break;
  }
#ifdef GLSL_DEBUG
  CheckGLError("SetUniformArray(%s,bool*)",name);
#endif
}
