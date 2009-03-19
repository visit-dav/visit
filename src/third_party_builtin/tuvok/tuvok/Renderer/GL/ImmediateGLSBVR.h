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
  \file    ImmediateGLSBVR.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
*/
#pragma once

#ifndef TUVOK_IMMEDIATE_GL_SBVR_H
#define TUVOK_IMMEDIATE_GL_SBVR_H

#include "GLSBVR.h"

class ImmediateGLSBVR : public GLSBVR {
  public:
    ImmediateGLSBVR(bool bUseOnlyPowerOfTwo, bool bDownSample,
                    bool bDisableBorder);
    virtual ~ImmediateGLSBVR() { }

    virtual void Paint();
    void Render();

    void Hack(float fov, float zNear, float zFar,
              float eye[3], float ref[3], float vup[3]);

  protected:
    /// This rendering path takes the raw data && TF; we don't want to do any
    /// sort of scaling inside the shader.
    virtual float CalculateScaling() const { return 1.0f; }

    virtual void SetViewPort(UINTVECTOR2 viLowerLeft,
                             UINTVECTOR2 viUpperRight);

  private:
    float fov;
    float z_near;
    float z_far;
    float eye[3];
    float ref[3];
    float vup[3];
};

#endif // TUVOK_IMMEDIATE_GL_SBVR_H
