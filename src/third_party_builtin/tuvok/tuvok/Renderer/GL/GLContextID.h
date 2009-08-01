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
  \file    GLContextID.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
*/
#pragma once

#ifndef TUVOK_GL_CONTEXT_ID_H
#define TUVOK_GL_CONTEXT_ID_H

#include <GL/glxew.h>
#include "../ContextID.h"

namespace tuvok {

/// Context wrapper based on GLX.  See base class for details.
class GLContextID : ContextID<GLContextID> {
  public:
    /// Create an ID with the current context.
    GLContextID() : ContextID<GLContextID>(), ctx(glXGetCurrentContext()) {} 
    /// Create an ID from the given context.
    GLContextID(GLXContext c) : ContextID<GLContextID>(), ctx(c) {}
    GLContextID(const GLContextID& ct) : ContextID<GLContextID>(),
                                         ctx(ct.ctx) {}

    static GLContextID Current() { return GLContextID(); }

    bool operator==(const GLContextID &gl_cid) const {
      return this->ctx == gl_cid.ctx;
    }
    bool operator!=(const GLContextID &gl_cid) const {
      return this->ctx != gl_cid.ctx;
    }

    GLContextID& operator=(const GLContextID &ct) {
      this->ctx = ct.ctx;
      return *this;
    }

  private:
    GLContextID(const ContextID<GLContextID>&); ///< unimplemented

  private:
    GLXContext ctx;
};

};

#endif // TUVOK_GL_CONTEXT_ID_H
