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
  \file    QtGLContext.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
*/
#pragma once

#ifndef TUVOK_QT_GL_CONTEXT_ID_H
#define TUVOK_QT_GL_CONTEXT_ID_H

#include "../ContextID.h"
#include "GL/glew.h"
#include <QtOpenGL/QGLContext>

namespace tuvok {

/// GL context information based on the `QGLContext' class.
class QtGLContextID : ContextID<QtGLContextID> {
  public:
    /// Create an ID with the current context.
    QtGLContextID() : ctx((QGLContext::currentContext())) {}
    /// Create an ID from the given context.
    QtGLContextID(const QGLContext *c) : ContextID<QtGLContextID>(), ctx(c) {}
    QtGLContextID(const QtGLContextID& ct) : ContextID<QtGLContextID>(),
                                             ctx(ct.ctx) {}

    static QtGLContextID Current() { return QtGLContextID(); }

    bool operator==(const QtGLContextID &gl_cid) const {
      return this->ctx == gl_cid.ctx;
    }
    bool operator!=(const QtGLContextID &gl_cid) const {
      return this->ctx != gl_cid.ctx;
    }

    QtGLContextID& operator=(const QtGLContextID &ct) {
      this->ctx = ct.ctx;
      return *this;
    }

  private:
    QtGLContextID(const ContextID<QtGLContextID>&); ///< unimplemented

  private:
    const QGLContext *ctx;
};

};
#endif // TUVOK_GL_CONTEXT_ID_H
