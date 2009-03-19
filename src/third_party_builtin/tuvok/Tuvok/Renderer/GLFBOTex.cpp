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
#include <stdio.h>
#include "GLFBOTex.h"
#include <Controller/MasterController.h>

#ifdef WIN32
  #ifndef DEBUG
    #pragma warning( disable : 4189 ) // disable unused var warning
  #endif
#endif


GLuint	GLFBOTex::m_hFBO = 0;
int		GLFBOTex::m_iCount = 0;
bool	GLFBOTex::m_bInitialized = true;

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
			m_pMasterController->DebugOut()->Error("GLFBOTex:GLFBOTex","failed to initialize GLEW!");
			return;
		}
		if (!glewGetExtension("GL_EXT_framebuffer_object")) {
			m_pMasterController->DebugOut()->Error("GLFBOTex:GLFBOTex","GL_EXT_framebuffer_object not supported!");
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

	if (GL_NO_ERROR!=glGetError()) {
    m_pMasterController->DebugOut()->Error("GLFBOTex:GLFBOTex","Error during creation!");
		glDeleteFramebuffersEXT(1,&m_hFBO);
		m_hFBO=0;
		return;
	}
	initTextures(minfilter,magfilter,wrapmode,width,height,intformat);
	if (GL_NO_ERROR!=glGetError()) {
    m_pMasterController->DebugOut()->Error("GLFBOTex:GLFBOTex","Error during texture init!");
		glDeleteTextures(m_iNumBuffers,m_hTexture);
		delete[] m_hTexture;
		m_hTexture=NULL;
		return;
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
		m_pMasterController->DebugOut()->Message("GLFBOTex:~GLFBOTex","FBO released via destructor call.");
		glDeleteFramebuffersEXT(1,&m_hFBO);
		m_hFBO=0;
	}
}



/**
 * Build a dummy texture according to the parameters.
 */
void GLFBOTex::initTextures(GLenum minfilter,GLenum magfilter,GLenum wrapmode, GLsizei width, GLsizei height, GLenum intformat) {
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
					glTexImage2D(GL_TEXTURE_2D, level, intformat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
					width/=2;
					height/=2;
					if (width==0 && height>0) width=1;
					if (width>0 && height==0) height=1;
					++level;
				} while (width>=1 && height>=1);
				break;
			default:
				glTexImage2D(GL_TEXTURE_2D, 0, intformat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
	}
}


/**
 * Build a new FBO.
 */
void GLFBOTex::initFBO(void) {
	m_pMasterController->DebugOut()->Message("GLFBOTex:initFBO","FBO initialized.");
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
			m_pMasterController->DebugOut()->Error("GLFBOTex:CheckFBO","%s() - Unsupported Format!",method); return false;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT: 
			m_pMasterController->DebugOut()->Error("GLFBOTex:CheckFBO","%s() - Incomplete attachment",method); return false;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT: 
			m_pMasterController->DebugOut()->Error("GLFBOTex:CheckFBO","%s() - Incomplete missing attachment",method); return false;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: 
			m_pMasterController->DebugOut()->Error("GLFBOTex:CheckFBO","%s() - Incomplete dimensions",method); return false;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: 
			m_pMasterController->DebugOut()->Error("GLFBOTex:CheckFBO","%s() - Incomplete formats",method); return false;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT: 
			m_pMasterController->DebugOut()->Error("GLFBOTex:CheckFBO","%s() - Incomplete draw buffer",method); return false;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT: 
			m_pMasterController->DebugOut()->Error("GLFBOTex:CheckFBO","%s() - Incomplete read buffer",method); return false;
		default:  return false;
	}	
}

// ************************************************************************************************************************************************************

int		VBOTex::m_iCount = 0;
bool	VBOTex::m_bInitialized = false;
bool	VBOTex::m_bPBOSupported = false;

VBOTex::VBOTex(MasterController* pMasterController, GLsizei width, GLsizei height, bool bHaveDepth, int iNumBuffers) :
  m_pMasterController(pMasterController),
  m_iSizeX(width),
  m_iSizeY(height),
  m_iSizePerElement(4*32) // GL_RGBA32F_ARB
{
	if (!m_bInitialized) {
		if (GLEW_OK!=glewInit()) {
			m_pMasterController->DebugOut()->Error("VBOTex:VBOTex","failed to initialize GLEW!");
			return;
		}
		m_bPBOSupported=(GL_FALSE!=glewGetExtension("GL_ARB_pixel_buffer_object"));
		if (!m_bPBOSupported) m_pMasterController->DebugOut()->Warning("VBOTex:VBOTex","GL_ARB_pixel_buffer_object not supported -> CPU fallback!");
		m_bInitialized=true;
	}
	assert(iNumBuffers>=0);
	assert(iNumBuffers<5);
	m_iNumBuffers=iNumBuffers;
	m_LastAttachment=new GLenum[m_iNumBuffers];
	for (int i=0; i<iNumBuffers; i++) m_LastAttachment[i]=GL_COLOR_ATTACHMENT0_EXT;
	// Generate FBO Texture
	m_hGLFBOTex = new GLFBOTex(m_pMasterController, GL_NEAREST,GL_NEAREST,GL_CLAMP_TO_EDGE,width,height,GL_RGBA32F_ARB,bHaveDepth);
	// Generate PBO -or- VBO and allocate data
	glGenBuffersARB(1,&m_hPBO);
	if (m_bPBOSupported) {
		glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, m_hPBO);
		glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB,width*height*4*sizeof(GLfloat),NULL,GL_STREAM_COPY_ARB);
		glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
	}
	else {
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_hPBO);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,width*height*4*sizeof(GLfloat),NULL,GL_STREAM_COPY_ARB);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	}
	m_iWidth=width;
	m_iHeight=height;
}

VBOTex::~VBOTex() {
	if (m_hGLFBOTex) {
		delete m_hGLFBOTex;
		m_hGLFBOTex=NULL;
	}
	if (m_LastAttachment) {
		delete[] m_LastAttachment;
		m_LastAttachment=NULL;
	}
	glDeleteBuffersARB(1,&m_hPBO);
}

/**
 * Lock texture for writing. Texture may not be bound any more!
 */
void GLFBOTex::Write(GLenum target, int iBuffer, bool bCheckBuffer) {
#ifdef _DEBUG
	if (!m_hFBO) {
		m_pMasterController->DebugOut()->Error("GLFBOTex:Write","FBO not initialized!");
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

void GLFBOTex::Read(GLenum texunit, int iBuffer) {
#ifdef _DEBUG
  if (m_LastTexUnit[iBuffer]!=0) {
    m_pMasterController->DebugOut()->Error("GLFBOTex:Read","Missing FinishRead()!");
  }
#endif
	assert(iBuffer>=0);
	assert(iBuffer<m_iNumBuffers);
	m_LastTexUnit[iBuffer]=texunit;
	glActiveTextureARB(texunit);
	glBindTexture(GL_TEXTURE_2D,m_hTexture[iBuffer]);
}

void GLFBOTex::ReadDepth(GLenum texunit) {
#ifdef _DEBUG
  if (m_LastDepthTextUnit!=0) {
    m_pMasterController->DebugOut()->Error("GLFBOTex:ReadDepth","Missing FinishDepthRead()!");
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

// Finish reading from this texture
void GLFBOTex::FinishRead(int iBuffer) {
	assert(iBuffer>=0);
	assert(iBuffer<m_iNumBuffers);
	glActiveTextureARB(m_LastTexUnit[iBuffer]);
	glBindTexture(GL_TEXTURE_2D,0);
	m_LastTexUnit[iBuffer]=0;
}

void VBOTex::Write(GLenum target,int iBuffer) {
	assert(iBuffer>=0);
	assert(iBuffer<m_iNumBuffers);
	m_hGLFBOTex->Write(target,iBuffer);
	m_LastAttachment[iBuffer]=target;
}

// Trouble: FinishWrite required for all buffers, but not all should end up in a VBO. Solution: CopyToVBO(int iBuffer);
// Trouble: need multiple VBOs...

void VBOTex::CopyToVBO(int iBuffer) {
	if (m_bPBOSupported) {
		glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT,m_hPBO);
		glReadBuffer(m_LastAttachment[iBuffer]);
		// copy to PBO
		glReadPixels(0,0,m_iWidth,m_iHeight,GL_RGBA,GL_FLOAT, NULL);
		// Set everything back to normal again
		glBindBufferARB(GL_PIXEL_PACK_BUFFER_EXT,0);
		glReadBuffer(GL_FRONT);
	}
	else {
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,m_hPBO);
		glReadBuffer(m_LastAttachment[iBuffer]);
		void *buffer=glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY);
		assert(buffer!=NULL);
		// copy to VBO (via CPU)
		glReadPixels(0,0,m_iWidth,m_iHeight,GL_RGBA,GL_FLOAT, buffer);	
		// Set everything back to normal again
		bool result=(GL_TRUE==glUnmapBufferARB(GL_ARRAY_BUFFER_ARB));
		assert(result!=GL_FALSE);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glReadBuffer(GL_FRONT);
	}
}

void VBOTex::FinishWrite(int iBuffer) {	
	m_hGLFBOTex->FinishWrite(iBuffer);
}


/// \todo More intelligent implementation: Render everything, then if a buffer is requested for reading, first
// perform VBO/PBO copy. Read(GLenum target,int iBuffer) does the trick. Should be copied to respective array.

void VBOTex::Read(void) {
	//assert(iBuffer>=0);
	//assert(iBuffer<m_iNumBuffers);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_hPBO);
	glVertexPointer(4,GL_FLOAT,0,NULL);
	glEnableClientState(GL_VERTEX_ARRAY);
};

void VBOTex::FinishRead(void) {
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void VBOTex::ReadTex(GLenum texunit,int iBuffer) {
	m_hGLFBOTex->Read(texunit,iBuffer);
}

// Finish rendering from this texture
void VBOTex::FinishReadTex(int iBuffer) {
	m_hGLFBOTex->FinishRead(iBuffer);
}
