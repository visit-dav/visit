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
  \file    GLTargetBinder.cpp
  \author    Jens Krueger
        SCI Institute
        University of Utah
  \date    Januray 2009
*/

#include "GLTargetBinder.h"
#include <Controller/Controller.h>


GLTargetBinder::GLTargetBinder(MasterController* pMasterController) : 
  m_pMasterController(pMasterController)
{
}

GLTargetBinder::~GLTargetBinder() {
  Unbind();
}


void GLTargetBinder::Bind(const std::vector<GLBufferID>& vpFBOs) {
  // now for the fun stuff with OpenGL binding
  
  UnbindInternal();
   
  switch (vpFBOs.size()) {
    case 1  : vpFBOs[0].pBuffer->Write(0,vpFBOs[0].iSubBuffer);
              GLFBOTex::OneDrawBuffer();
              break;
    case 2  : vpFBOs[0].pBuffer->Write(0,vpFBOs[0].iSubBuffer);
              vpFBOs[1].pBuffer->Write(1,vpFBOs[1].iSubBuffer);
              GLFBOTex::TwoDrawBuffers();
              break;
    case 3  : vpFBOs[0].pBuffer->Write(0,vpFBOs[0].iSubBuffer);
              vpFBOs[1].pBuffer->Write(1,vpFBOs[1].iSubBuffer);
              vpFBOs[2].pBuffer->Write(2,vpFBOs[2].iSubBuffer);
              GLFBOTex::ThreeDrawBuffers();
              break;
    case 4  : vpFBOs[0].pBuffer->Write(0,vpFBOs[0].iSubBuffer);
              vpFBOs[1].pBuffer->Write(1,vpFBOs[1].iSubBuffer);
              vpFBOs[2].pBuffer->Write(2,vpFBOs[2].iSubBuffer);
              vpFBOs[3].pBuffer->Write(3,vpFBOs[3].iSubBuffer);
              GLFBOTex::FourDrawBuffers();
              break;
    default : T_ERROR( "Invalid number of FBOs %i", int(vpFBOs.size()));
              return;
  }

  m_vpBoundFBOs = vpFBOs;
}

void GLTargetBinder::Bind(GLFBOTex* pFBO0, int iSubBuffer0, 
                          GLFBOTex* pFBO1, int iSubBuffer1, 
                          GLFBOTex* pFBO2, int iSubBuffer2, 
                          GLFBOTex* pFBO3, int iSubBuffer3) {
  std::vector<GLBufferID> vpFBOs;
  vpFBOs.push_back(GLBufferID(pFBO0, iSubBuffer0));
  if (pFBO1) vpFBOs.push_back(GLBufferID(pFBO1, iSubBuffer1));
  if (pFBO2) vpFBOs.push_back(GLBufferID(pFBO2, iSubBuffer2));
  if (pFBO3) vpFBOs.push_back(GLBufferID(pFBO3, iSubBuffer3));

  Bind(vpFBOs);
}

void GLTargetBinder::Bind(GLFBOTex* pFBO0, GLFBOTex* pFBO1, GLFBOTex* pFBO2, GLFBOTex* pFBO3) {
  std::vector<GLBufferID> vpFBOs;
  vpFBOs.push_back(GLBufferID(pFBO0));
  if (pFBO1) vpFBOs.push_back(GLBufferID(pFBO1));
  if (pFBO2) vpFBOs.push_back(GLBufferID(pFBO2));
  if (pFBO3) vpFBOs.push_back(GLBufferID(pFBO3));

  Bind(vpFBOs);
}

void GLTargetBinder::Unbind() {
  UnbindInternal();
  GLFBOTex::OneDrawBuffer();
}

void GLTargetBinder::UnbindInternal() {
 
  if (m_vpBoundFBOs.size() == 1) {
    m_vpBoundFBOs[0].pBuffer->FinishWrite(m_vpBoundFBOs[0].iSubBuffer);
  } else
  if (m_vpBoundFBOs.size() == 2) {
    GLFBOTex::NoDrawBuffer();
    m_vpBoundFBOs[1].pBuffer->FinishWrite(m_vpBoundFBOs[1].iSubBuffer);
    m_vpBoundFBOs[0].pBuffer->FinishWrite(m_vpBoundFBOs[0].iSubBuffer);
  } else
  if (m_vpBoundFBOs.size() == 3) {
    GLFBOTex::NoDrawBuffer();
    m_vpBoundFBOs[2].pBuffer->FinishWrite(m_vpBoundFBOs[2].iSubBuffer);
    m_vpBoundFBOs[1].pBuffer->FinishWrite(m_vpBoundFBOs[1].iSubBuffer);
    m_vpBoundFBOs[0].pBuffer->FinishWrite(m_vpBoundFBOs[0].iSubBuffer);
  } else
  if (m_vpBoundFBOs.size() == 3) {
    GLFBOTex::NoDrawBuffer();
    m_vpBoundFBOs[3].pBuffer->FinishWrite(m_vpBoundFBOs[3].iSubBuffer);
    m_vpBoundFBOs[2].pBuffer->FinishWrite(m_vpBoundFBOs[2].iSubBuffer);
    m_vpBoundFBOs[1].pBuffer->FinishWrite(m_vpBoundFBOs[1].iSubBuffer);
    m_vpBoundFBOs[0].pBuffer->FinishWrite(m_vpBoundFBOs[0].iSubBuffer);
  }  
  
  m_vpBoundFBOs.clear();
}
