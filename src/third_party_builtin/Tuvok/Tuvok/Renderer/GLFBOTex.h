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
  \file    GPUMemMan.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
           Jens Schneider
           tum.3D, Muenchen
  \date    August 2008
*/
#ifndef GLFBOTEX_H_
#define GLFBOTEX_H_

#include <assert.h>
#include <stdlib.h>
#include "GLObject.h"

class MasterController;

class GLFBOTex : public GLObject {
public:
	GLFBOTex(MasterController* pMasterController, GLenum minfilter, GLenum magfilter, GLenum wrapmode, GLsizei width, GLsizei height, GLenum intformat, unsigned int iSizePerElement, bool bHaveDepth=false, int iNumBuffers=1);
	virtual ~GLFBOTex(void);
	virtual void Write(GLenum target=GL_COLOR_ATTACHMENT0_EXT, int iBuffer=0, bool bCheckBuffer=true);
	virtual void Read(GLenum texunit,int iBuffer=0);
	virtual void FinishWrite(int iBuffer=0);
	virtual void FinishRead(int iBuffer=0);
  virtual void ReadDepth(GLenum texunit);
  virtual void FinishDepthRead();
	virtual operator GLuint(void) { return m_hTexture[0]; }
	virtual operator GLuint*(void) { return m_hTexture; }

  /// \todo check how much mem an FBO really occupies
  virtual UINT64 GetCPUSize() {return m_iNumBuffers*m_iSizeX*m_iSizeY*m_iSizePerElement/8 + ((m_hDepthBuffer) ? m_iSizeX*m_iSizeY*4 : 0);}
  virtual UINT64 GetGPUSize() {return GetCPUSize();}
  
  static void NoDrawBuffer() {glDrawBuffer(GL_NONE);}
  static void OneDrawBuffer() {glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);}
  static void TwoDrawBuffers() {
    GLenum twobuffers[]  = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
    glDrawBuffers(2, twobuffers);
  }
  
private:
  MasterController    *m_pMasterController;
  unsigned int        m_iSizePerElement;
  GLuint              m_iSizeX;
  GLuint              m_iSizeY;

  bool			CheckFBO(const char* method);
	void			initFBO(void);
	void			initTextures(GLenum minfilter, GLenum magfilter, GLenum wrapmode, GLsizei width, GLsizei height, GLenum intformat);
	GLuint			        *m_hTexture;
	GLuint			        m_hDepthBuffer;
	static GLuint	      m_hFBO;
	static bool		      m_bInitialized;
	static int		      m_iCount;
	GLenum			        *m_LastTexUnit;
	GLenum			        m_LastDepthTextUnit;
	int				          m_iNumBuffers;
	GLenum			        *m_LastAttachment;
};


// ************************************************************************************************************************************************************

class VBOTex : public GLObject {
public:
	VBOTex(MasterController* pMasterController, GLsizei width, GLsizei height, bool bHaveDepth=false, int iNumBuffers=1);
	virtual ~VBOTex(void);
	virtual void CopyToVBO(int iBuffer=0);
	virtual void Write(GLenum target=GL_COLOR_ATTACHMENT0_EXT, int iBuffer=0);
	virtual void FinishWrite(int iBuffer=0);
	virtual void ReadTex(GLenum texunit, int iBuffer=0);
	virtual void FinishReadTex(int iBuffer=0);
	virtual void Read(void);
	virtual void FinishRead(void);
	virtual operator GLuint(void) { return m_hPBO; }	

  /// \todo check how much mem an FBO really occupies
  virtual UINT64 GetCPUSize() {return m_iSizeX*m_iSizeY*m_iSizePerElement/8;}
  /// \todo check how much mem an FBO really occupies
  virtual UINT64 GetGPUSize() {return m_iSizeX*m_iSizeY*m_iSizePerElement/8;}

private:
  MasterController *m_pMasterController;
  GLuint            m_iSizeX;
  GLuint            m_iSizeY;
  unsigned int      m_iSizePerElement;

	GLFBOTex          *m_hGLFBOTex;
	GLuint		        m_hPBO;
	GLsizei		        m_iWidth;
	GLsizei		        m_iHeight;
	GLenum		        *m_LastAttachment;
	static int	      m_iCount;
	static bool	      m_bInitialized;
	static bool	      m_bPBOSupported;
	int			          m_iNumBuffers;
};


#endif  // GLFBOTEX_H_
