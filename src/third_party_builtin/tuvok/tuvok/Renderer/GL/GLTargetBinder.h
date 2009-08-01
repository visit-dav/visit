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
  \file    GLTargetBinder.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    Januray 2009
*/
#pragma once

#ifndef GLTARGETBINDER_H
#define GLTARGETBINDER_H

#include <vector>
#include "../../StdTuvokDefines.h"
class GLFBOTex;
class MasterController;

class GLBufferID {
public:
  GLBufferID(GLFBOTex* _pBuffer, UINT32 _iSubBuffer=0) :
    pBuffer(_pBuffer),
    iSubBuffer(_iSubBuffer)
  { }

  GLFBOTex* pBuffer;
  UINT32 iSubBuffer;
};

class GLTargetBinder {
  public:
    GLTargetBinder(MasterController* pMasterController);
    virtual ~GLTargetBinder();

    virtual void Bind(const std::vector<GLBufferID>& vpFBOs);
    virtual void Bind(GLFBOTex* pFBO0, GLFBOTex* pFBO1 = NULL, GLFBOTex* pFBO2 = NULL, GLFBOTex* pFBO3 = NULL);
    virtual void Bind(GLFBOTex* pFBO0, int iSubBuffer0, 
                      GLFBOTex* pFBO1 = NULL, int iSubBuffer1=0, 
                      GLFBOTex* pFBO2 = NULL, int iSubBuffer2=0, 
                      GLFBOTex* pFBO3 = NULL, int iSubBuffer3=0);

    virtual void Unbind();

  protected:
    MasterController*        m_pMasterController;
    std::vector<GLBufferID>  m_vpBoundFBOs;

    void UnbindInternal();
};
#endif // GLTARGETBINDER_H
