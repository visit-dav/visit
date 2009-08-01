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
  \file    GLFBOTex.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
           Jens Schneider
           tum.3D, Muenchen
  \date    August 2008
*/
#include "GLFBOTex.h"
#include <Controller/Controller.h>

#ifdef WIN32
  #ifndef DEBUG
    #pragma warning( disable : 4189 ) // disable unused var warning
  #endif
#endif


GLuint  GLFBOTex::m_hFBO = 0;
int    GLFBOTex::m_iCount = 0;
bool  GLFBOTex::m_bInitialized = true;

/**
 * Constructor: on first instantiation, generate an FBO.
 * In any case a new dummy texture according to the parameters is generated.
 */

GLFBOTex::GLFBOTex(MasterController* pMasterController, GLenum minfilter, GLenum magfilter, GLenum wrapmode, GLsizei width, GLsizei height, GLenum intformat, unsigned int iSizePerElement, bool bHaveDepth, int iNumBuffers) :
  m_pMasterController(pMasterController),
  m_iSizePerElement(iSizePerElement),
  m_iSizeX(width),
  m_iSizeY(height),
  m_LastDepthTextUnit(0),
  m_iNumBuffers(iNumBuffers)
{
  if (width<1) width=1;
  if (height<1) height=1;
  if (!m_bInitialized) {
    if (GLEW_OK!=glewInit()) {
      T_ERROR("failed to initialize GLEW!");
      return;
    }
    if (!glewGetExtension("GL_EXT_framebuffer_object")) {
      T_ERROR("GL_EXT_framebuffer_object not supported!");
      return;
    }
    m_bInitialized=true;
  }
  glGetError();
  assert(iNumBuffers>0);
  assert(iNumBuffers<5);
  m_hTexture=new GLuint[iNumBuffers];
  m_LastTexUnit=new GLenum[iNumBuffers];
  m_LastAttachment=new GLenum[iNumBuffers];
  for (int i=0; i<m_iNumBuffers; i++) {
    m_LastTexUnit[i]=0;
    m_LastAttachment[i]=GL_COLOR_ATTACHMENT0_EXT+i;
    m_hTexture[i]=0;
  }
  if (m_hFBO==0)
    initFBO();

  {
    GLenum glerr = glGetError();
    if(GL_NO_ERROR != glerr) {
      T_ERROR("Error '%d' during FBO creation!", static_cast<int>(glerr));
      glDeleteFramebuffersEXT(1,&m_hFBO);
      m_hFBO=0;
      return;
    }
  }
  initTextures(minfilter,magfilter,wrapmode,width,height,intformat);
  {
    GLenum glerr = glGetError();
    if(GL_NO_ERROR != glerr) {
      T_ERROR("Error '%d' during texture creation!", static_cast<int>(glerr));
      glDeleteTextures(m_iNumBuffers,m_hTexture);
      delete[] m_hTexture;
      m_hTexture=NULL;
      return;
    }
  }
  if (bHaveDepth) {
#ifdef GLFBOTEX_DEPTH_RENDERBUFFER
    glGenRenderbuffersEXT(1,&m_hDepthBuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,width,height);
#else
    glGenTextures(1,&m_hDepthBuffer);
    glBindTexture(GL_TEXTURE_2D,m_hDepthBuffer);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,width,height,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
#endif
  }
  else m_hDepthBuffer=0;

  ++m_iCount;
}


/**
 * Destructor: Delete texture object. If no more instances of
 * GLFBOTex are around, the FBO is deleted as well.
 */
GLFBOTex::~GLFBOTex(void) {
  if (m_hTexture) {
    glDeleteTextures(m_iNumBuffers,m_hTexture);
    delete[] m_hTexture;
    m_hTexture=NULL;
  }
  if (m_LastTexUnit) {
    delete[] m_LastTexUnit;
    m_LastTexUnit=NULL;
  }
  if (m_LastAttachment) {
    delete[] m_LastAttachment;
    m_LastAttachment=NULL;
  }
#ifdef GLFBOTEX_DEPTH_RENDERBUFFER
  if (m_hDepthBuffer) glDeleteRenderbuffersEXT(1,&m_hDepthBuffer);
#else
  if (m_hDepthBuffer) glDeleteTextures(1,&m_hDepthBuffer);
#endif
  m_hDepthBuffer=0;
  --m_iCount;
  if (m_iCount==0) {
    m_pMasterController->DebugOut()->Message(_func_, "FBO released via "
                                                     "destructor call.");
    glDeleteFramebuffersEXT(1,&m_hFBO);
    m_hFBO=0;
  }
}



/**
 * Build a dummy texture according to the parameters.
 */
void GLFBOTex::initTextures(GLenum minfilter,GLenum magfilter,GLenum wrapmode, GLsizei width, GLsizei height, GLenum intformat) {
  MESSAGE("Initializing 2D texture of dimensions: %ux%u",
          static_cast<unsigned int>(width), static_cast<unsigned int>(height));
  glDeleteTextures(m_iNumBuffers,m_hTexture);
  glGenTextures(m_iNumBuffers,m_hTexture);
  for (int i=0; i<m_iNumBuffers; i++) {
    glBindTexture(GL_TEXTURE_2D, m_hTexture[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapmode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapmode);
    int level=0;
    switch (minfilter) {
      case GL_NEAREST_MIPMAP_NEAREST:
      case GL_LINEAR_MIPMAP_NEAREST:
      case GL_NEAREST_MIPMAP_LINEAR:
      case GL_LINEAR_MIPMAP_LINEAR:
        do {
          glTexImage2D(GL_TEXTURE_2D, level, intformat, width, height, 0,
                       GL_RGBA, GL_UNSIGNED_BYTE, NULL);
          width/=2;
          height/=2;
          if (width==0 && height>0) width=1;
          if (width>0 && height==0) height=1;
          ++level;
        } while (width>=1 && height>=1);
        break;
      default:
        glTexImage2D(GL_TEXTURE_2D, 0, intformat, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
  }
}


/**
 * Build a new FBO.
 */
void GLFBOTex::initFBO(void) {
  MESSAGE("Initializing FBO...");
  glGenFramebuffersEXT(1, &m_hFBO);
}

/**
 * Check the FBO for consistency.
 */
bool GLFBOTex::CheckFBO(const char* method) {
  GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  switch(status) {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
      return true;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      T_ERROR("%s() - Unsupported Format!",method); return false;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
      T_ERROR("%s() - Incomplete attachment",method); return false;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      T_ERROR("%s() - Incomplete missing attachment",method); return false;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      T_ERROR("%s() - Incomplete dimensions",method); return false;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      T_ERROR("%s() - Incomplete formats",method); return false;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      T_ERROR("%s() - Incomplete draw buffer",method); return false;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      T_ERROR("%s() - Incomplete read buffer",method); return false;
    default:  return false;
  }
}

/**
 * Lock texture for writing. Texture may not be bound any more!
 */
void GLFBOTex::Write(unsigned int iTargetBuffer, int iBuffer, bool bCheckBuffer) {
  GLenum target = GL_COLOR_ATTACHMENT0_EXT + iTargetBuffer;

#ifdef _DEBUG
  if (!m_hFBO) {
    T_ERROR("FBO not initialized!");
    return;
  }
#endif

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,m_hFBO);
  assert(iBuffer>=0);
  assert(iBuffer<m_iNumBuffers);
  m_LastAttachment[iBuffer]=target;
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, target, GL_TEXTURE_2D, m_hTexture[iBuffer], 0);
  if (m_hDepthBuffer) glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_hDepthBuffer,0);
  if (bCheckBuffer) {
#ifdef _DEBUG
  if (!CheckFBO("Write")) return;
#endif
  }
}

void GLFBOTex::FinishWrite(int iBuffer) {
  glGetError();
  assert(iBuffer>=0);
  assert(iBuffer<m_iNumBuffers);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,m_LastAttachment[iBuffer],GL_TEXTURE_2D,0,0);
  if (m_hDepthBuffer) glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,0,0);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
}

void GLFBOTex::Read(unsigned int iTargetUnit, int iBuffer) {
  GLenum texunit = GL_TEXTURE0 + iTargetUnit;
#ifdef _DEBUG
  if (m_LastTexUnit[iBuffer]!=0) {
    T_ERROR("Missing FinishRead()!");
  }
#endif
  assert(iBuffer>=0);
  assert(iBuffer<m_iNumBuffers);
  m_LastTexUnit[iBuffer]=texunit;
  glActiveTextureARB(texunit);
  glBindTexture(GL_TEXTURE_2D,m_hTexture[iBuffer]);
}

void GLFBOTex::ReadDepth(unsigned int iTargetUnit) {
  GLenum texunit = GL_TEXTURE0 + iTargetUnit;
#ifdef _DEBUG
  if (m_LastDepthTextUnit!=0) {
    T_ERROR("Missing FinishDepthRead()!");
  }
#endif
  m_LastDepthTextUnit=texunit;
  glActiveTextureARB(texunit);
  glBindTexture(GL_TEXTURE_2D,m_hDepthBuffer);
}

// Finish reading from the depth texture
void GLFBOTex::FinishDepthRead() {
  glActiveTextureARB(m_LastDepthTextUnit);
  glBindTexture(GL_TEXTURE_2D,0);
  m_LastDepthTextUnit=0;
}

void GLFBOTex::NoDrawBuffer() {
  glDrawBuffer(GL_NONE);
}

void GLFBOTex::OneDrawBuffer() {
  glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
}

void GLFBOTex::TwoDrawBuffers() {
  GLenum twobuffers[]  = { GL_COLOR_ATTACHMENT0_EXT,
                           GL_COLOR_ATTACHMENT1_EXT };
  glDrawBuffers(2, twobuffers);
}

void GLFBOTex::ThreeDrawBuffers() {
  GLenum threebuffers[]  = { GL_COLOR_ATTACHMENT0_EXT,
                            GL_COLOR_ATTACHMENT1_EXT,
                            GL_COLOR_ATTACHMENT2_EXT};
  glDrawBuffers(3, threebuffers);
}

void GLFBOTex::FourDrawBuffers() {
  GLenum fourbuffers[]  = { GL_COLOR_ATTACHMENT0_EXT,
                            GL_COLOR_ATTACHMENT1_EXT,
                            GL_COLOR_ATTACHMENT2_EXT,
                            GL_COLOR_ATTACHMENT3_EXT};
  glDrawBuffers(4, fourbuffers);
}

// Finish reading from this texture
void GLFBOTex::FinishRead(int iBuffer) {
  assert(iBuffer>=0);
  assert(iBuffer<m_iNumBuffers);
  glActiveTextureARB(m_LastTexUnit[iBuffer]);
  glBindTexture(GL_TEXTURE_2D,0);
  m_LastTexUnit[iBuffer]=0;
}
