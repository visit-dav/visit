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
  \file    GLFBOTex.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
           Jens Schneider
           tum.3D, Muenchen
  \date    August 2008
*/
#ifndef GLFBOTEX_H_
#define GLFBOTEX_H_

#include "../../StdTuvokDefines.h"
#include <assert.h>
#include <stdlib.h>
#include "GLObject.h"

class MasterController;

class GLFBOTex : public GLObject {
public:
  GLFBOTex(MasterController* pMasterController, GLenum minfilter, GLenum magfilter, GLenum wrapmode, GLsizei width, GLsizei height, GLenum intformat, unsigned int iSizePerElement, bool bHaveDepth=false, int iNumBuffers=1);
  virtual ~GLFBOTex(void);
  virtual void Write(unsigned int iTargetBuffer=0, int iBuffer=0, bool bCheckBuffer=true);
  virtual void Read(unsigned int iTargetUnit,int iBuffer=0);
  virtual void FinishWrite(int iBuffer=0);
  virtual void FinishRead(int iBuffer=0);
  virtual void ReadDepth(unsigned int iTargetUnit);
  virtual void FinishDepthRead();
  virtual operator GLuint(void) { return m_hTexture[0]; }
  virtual operator GLuint*(void) { return m_hTexture; }

  /// \todo check how much mem an FBO really occupies
  virtual UINT64 GetCPUSize() {return m_iNumBuffers*m_iSizeX*m_iSizeY*m_iSizePerElement + ((m_hDepthBuffer) ? m_iSizeX*m_iSizeY*4 : 0);}
  virtual UINT64 GetGPUSize() {return GetCPUSize();}

  static void NoDrawBuffer() { glDrawBuffer(GL_NONE); }
  static void OneDrawBuffer() { glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT); }
  static void TwoDrawBuffers() {
    GLenum twobuffers[]  = { GL_COLOR_ATTACHMENT0_EXT,
                             GL_COLOR_ATTACHMENT1_EXT };
    glDrawBuffers(2, twobuffers);
  }
  static void ThreeDrawBuffers() {
    GLenum threebuffers[]  = { GL_COLOR_ATTACHMENT0_EXT,
                              GL_COLOR_ATTACHMENT1_EXT,
                              GL_COLOR_ATTACHMENT2_EXT};
    glDrawBuffers(3, threebuffers);
  }
  static void FourDrawBuffers() {
    GLenum fourbuffers[]  = { GL_COLOR_ATTACHMENT0_EXT,
                              GL_COLOR_ATTACHMENT1_EXT,
                              GL_COLOR_ATTACHMENT2_EXT,
                              GL_COLOR_ATTACHMENT3_EXT};
    glDrawBuffers(4, fourbuffers);
  }



private:
  MasterController*   m_pMasterController;
  unsigned int        m_iSizePerElement;
  GLuint              m_iSizeX;
  GLuint              m_iSizeY;
  GLuint*             m_hTexture;
  GLuint              m_hDepthBuffer;
  static GLuint       m_hFBO;
  static bool         m_bInitialized;
  static int          m_iCount;
  GLenum*             m_LastTexUnit;
  GLenum              m_LastDepthTextUnit;
  int                 m_iNumBuffers;
  GLenum*             m_LastAttachment;

  bool                CheckFBO(const char* method);
  void                initFBO(void);
  void                initTextures(GLenum minfilter, GLenum magfilter, GLenum wrapmode, GLsizei width, GLsizei height, GLenum intformat);
};


#endif  // GLFBOTEX_H_
