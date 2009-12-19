//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2008 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
//    File   : gldefs.h
//    Author : Martin Cole
//    Date   : Fri Dec 29 11:26:44 2006

#if ! defined(gldefs_h)
#define gldefs_h

#include <GL/glew.h>

#if defined(OGL_DBG)
# if !defined(_WIN32)
#  define CHECK_OPENGL_ERROR()                                       \
   {                                                                    \
    GLenum errCode;                                                     \
    int cnt = 1;                                                        \
    while ((errCode = glGetError()) != GL_NO_ERROR) {                   \
      std::cerr<< "OpenGL Error(" << cnt << "):"                        \
          << __FILE__ << ":" << __LINE__ << " '"                        \
          << (const char*)gluErrorString(errCode) << "'" << std::endl;  \
    }                                                                   \
  }
# else
   // Note, this does not use the "while" loop because on Windows, the
   // while can cause an infinite loop.  This might happen when OpenGL
   // is not initialized, but I'm not sure.  At some point, if someone
   // verifies that the While loop works under !Windows, then we
   // should move a different version of this macro definition into
   // the Windows and non-windows areas.
#  define CHECK_OPENGL_ERROR()                                   \
     {                                                                \
       GLenum errCode = glGetError();                                 \
       if( errCode != GL_NO_ERROR ) {                                 \
          std::cerr << "OpenGL Error( #" << errCode << " ):"          \
                    << __FILE__ << ":" << __LINE__ << " - '"          \
                    << (const char*)gluErrorString(errCode) << "'\n"; \
       }                                                              \
     }
# endif
#else
#  define CHECK_OPENGL_ERROR() 
#endif

#if defined(OGL_DBG)
#  define CHECK_FRAMEBUFFER_STATUS()                                  \
{                                                                     \
  GLenum status;                                                      \
  status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);           \
  switch(status) {                                                    \
  case GL_FRAMEBUFFER_COMPLETE_EXT:                                   \
    break;                                                            \
  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:                      \
    cerr << __FILE__ << ":" << __LINE__ << " - "                      \
         << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT"                \
	 << endl;                                                     \
    break;                                                            \
  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:              \
    cerr << __FILE__ << ":" << __LINE__ << " - "                      \
         << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT"        \
	 << endl;                                                     \
    break;                                                            \
  case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:                      \
    cerr << __FILE__ << ":" << __LINE__ << " - "                      \
         << "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT" << endl;       \
    break;                                                            \
  case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:                         \
    cerr << __FILE__ << ":" << __LINE__ << " - "                      \
         << "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT" << endl;          \
    break;                                                            \
  case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:                     \
    cerr << __FILE__ << ":" << __LINE__ << " - "                      \
         << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT" << endl;      \
    break;                                                            \
  case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:                     \
    cerr << __FILE__ << ":" << __LINE__ << " - "                      \
         << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT" << endl;      \
    break;                                                            \
  case GL_FRAMEBUFFER_UNSUPPORTED_EXT:                                \
    cerr << __FILE__ << ":" << __LINE__ << " - "                      \
         << "GL_FRAMEBUFFER_UNSUPPORTED_EXT" << endl;                 \
    break;                                                            \
  default:                                                            \
    /* programming error; will fail on all hardware */                \
    assert(0);                                                        \
  }                                                                   \
}                                                                     \

#else
#  define CHECK_FRAMEBUFFER_STATUS() 
#endif

#endif
