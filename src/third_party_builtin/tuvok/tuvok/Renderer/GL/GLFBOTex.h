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
#ifndef TUVOK_GLFBOTEX_H_
#define TUVOK_GLFBOTEX_H_

#include "../../StdTuvokDefines.h"
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
  virtual UINT64 GetCPUSize() {return EstimateCPUSize(m_iSizeX, m_iSizeY, m_iSizePerElement, m_hDepthBuffer!=0, m_iNumBuffers);}
  virtual UINT64 GetGPUSize() {return EstimateGPUSize(m_iSizeX, m_iSizeY, m_iSizePerElement, m_hDepthBuffer!=0, m_iNumBuffers);}

  static UINT64 EstimateCPUSize(GLsizei width, GLsizei height, unsigned int iSizePerElement, bool bHaveDepth=false, int iNumBuffers=1) {return iNumBuffers*width*height*iSizePerElement + ((bHaveDepth) ? width*height*4 : 0);}
  static UINT64 EstimateGPUSize(GLsizei width, GLsizei height, unsigned int iSizePerElement, bool bHaveDepth=false, int iNumBuffers=1) {return EstimateCPUSize(width, height, iSizePerElement, bHaveDepth, iNumBuffers);}


  static void NoDrawBuffer();
  static void OneDrawBuffer();
  static void TwoDrawBuffers();
  static void ThreeDrawBuffers();
  static void FourDrawBuffers();

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
#endif  // TUVOK_GLFBOTEX_H_
