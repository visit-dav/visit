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
  \file    Context.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   Includes the proper context code and typedefs the compile-time
           context to the appropriate one.
*/
#pragma once
#ifndef TUVOK_CONTEXT_H
#define TUVOK_CONTEXT_H

// We also have a `GLContextID' which is not based on Qt.  For most projects,
// we can rely on Qt's presence, so it's better to use the Qt object for this.
// If you're embedding Tuvok in another app though, you might want to replace
// the Qt context implementation with the straight-GL implementation.
#ifdef USE_DIRECTX
# include "DX/DXContextID.h"
  namespace tuvok {
    typedef DXContextID CTContext;
  };
#elif defined(TUVOK_NO_QT)
# include "GL/GLContextID.h"
  namespace tuvok {
    typedef GLContextID CTContext;
  };
#else
# include "GL/QtGLContextID.h"
  namespace tuvok {
    typedef QtGLContextID CTContext;
  };
#endif

#endif // TUVOK_CONTEXT_H
