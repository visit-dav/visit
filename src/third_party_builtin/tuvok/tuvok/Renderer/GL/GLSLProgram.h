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
  \file    GLSLProgram.h
  \author  Jens Schneider, Jens Krueger
           SCI Institute, University of Utah
  \date    October 2008
*/
#pragma once

#ifndef GLSLPROGRAM_H
#define GLSLPROGRAM_H

/// if enabled, SetUniformVector allows for implicit casting, which can
/// impact performance
#define GLSL_ALLOW_IMPLICIT_CASTS
/// if enabled, GLSL-compiler warnings are treated as errors
#define GLSLPROGRAM_STRICT

#ifdef _DEBUG
  /// switches on debugging output - can be changed per-class.
  #define GLSL_DEBUG
#endif

/// on windows warn if GLSL_DEBUG differs from _DEBUG (i.e.. if the
/// lines above are not used)
#ifdef _WIN32
  #ifndef _DEBUG
    #ifdef GLSL_DEBUG
      #pragma message("    [GLSLProgram.h] DEBUG ON.\n")
    #endif
  #else
    #ifndef GLSL_DEBUG
      #pragma message("    [GLSLProgram.h] DEBUG OFF.\n")
    #endif
  #endif
#endif

//! Used to specify sources for shaders.
/*! \author Jens Schneider
    \date March 2004 */
typedef enum {
  GLSLPROGRAM_DISK=0, //!< the shader is a C-string containing the file name.
  GLSLPROGRAM_STRING  /*!< the shader sourcecode is specified directly in the
                           given C-string. */
} GLSLPROGRAM_SOURCE;

#include "../../StdTuvokDefines.h"
#include "GLObject.h"

class MasterController;

/**
 * Wrapper for handling OpenGL 2.0 conformant program objects.
 * \class GLSLProgram
 * \version 1.0c
 * \warning include _before_ you include anything like gl.h, glut.h etc.
 * \warning requires the GL Extension Wrangler (GLEW) library.
 * \author <a href="mailto:jens.schneider@in.tum.de">Jens Schneider</a>
 * \date November 2005
 */
class GLSLProgram : public GLObject {
public:
  /// Standard Constructor.
  GLSLProgram(MasterController* pMasterController);
  /// Constructor. Loads any combination of vertex (VPFile) and
  /// fragment (FPFile) shaders either from disk or from a C-string.
  GLSLProgram(MasterController* pMasterController,
              const char *VSFile, const char *FSFile,
              GLSLPROGRAM_SOURCE src = GLSLPROGRAM_DISK);
  virtual ~GLSLProgram();

  /// Loads any combination of vertex (VPFile) and fragment (FPFile)
  /// shaders either from disk or from a C-string.
  void Load(const char *VSFile, const char *FSFile,
            GLSLPROGRAM_SOURCE src=GLSLPROGRAM_DISK);
  /// Enables this shader for rendering.
  void Enable(void);
  /// Disables this shader for rendering (using fixed function pipeline again)
  void Disable(void);

  /// Returns the handle of this shader.
  operator GLuint(void) const;
  /// returns true if this program is valid
  bool IsValid(void) const;

  /// Sets an uniform parameter.
  void SetUniformVector(const char *name,
                        float x=0.0f, float y=0.0f,
                        float z=0.0f, float w=0.0f) const;
  /// Sets an uniform parameter.
  void SetUniformVector(const char *name,
                        int x=0, int y=0, int z=0, int w=0) const;
  /// Sets an uniform parameter.
  void SetUniformVector(const char *name,
                        bool x=false, bool y=false,
                        bool z=false, bool w=false) const;
  /// Sets an uniform parameter.
  void SetUniformVector(const char *name, const float *v) const;
  /// Sets an uniform parameter.
  void SetUniformVector(const char *name, const int *i) const;
  /// Sets an uniform parameter.
  void SetUniformVector(const char *name, const bool *b) const;

  /// Sets an uniform matrix. Matrices are always float.
  void SetUniformMatrix(const char *name, const float *m,
                        bool bTranspose=false) const;

#ifdef GLSL_ALLOW_IMPLICIT_CASTS
  /// Sets an uniform matrix. Matrices are always float.
  void SetUniformMatrix(const char *name, const int *m,
                        bool bTranspose=false) const;
  /// Sets an uniform matrix. Matrices are always float.
  void SetUniformMatrix(const char *name, const bool *m,
                        bool bTranspose=false) const;
#endif

  /// Sets an uniform array. User has to take care that a is large enough.
  void SetUniformArray(const char *name, const float *a) const;
  /// Sets an uniform array. User has to take care that a is large enough.
  void SetUniformArray(const char *name, const int   *a) const;
  /// Sets an uniform array. User has to take care that a is large enough.
  void SetUniformArray(const char *name, const bool  *a) const;

  /// assume near zero CPU memory cost for shaders to avoid any memory
  /// manager from paging out shaders, the 1 is basically only to
  /// detect memory leaks
  virtual UINT64 GetCPUSize() {return 1;}
  /// assume near zero GPU memory cost for shaders to avoid any memory
  /// manager from paging out shaders, the 1 is basically only to
  /// detect memory leaks
  virtual UINT64 GetGPUSize() {return 1;}

  static bool         m_bGLUseARB;

private:
  bool    Initialize(void);
  GLuint  LoadShader(const char*, GLenum, GLSLPROGRAM_SOURCE src);
  bool    WriteInfoLog(const char*, GLuint, bool);
  bool    WriteError(GLhandleARB hObject);
  bool    CheckGLError(const char *pcError=NULL,
                       const char *pcAdditional=NULL) const;
  MasterController*   m_pMasterController;
  bool                m_bInitialized;
  bool                m_bEnabled;
  GLuint              m_hProgram;
  static bool         m_bGlewInitialized;
  static bool         m_bGLChecked;
  std::string         m_sVS;
  std::string         m_sFS;
};

#ifdef GLSL_ALLOW_IMPLICIT_CASTS
#undef GLSL_ALLOW_IMPLICIT_CASTS
#endif

#endif // GLSLPROGRAM_H
