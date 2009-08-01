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
  \file    ContextID.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   Base class for holding comparative context information.
*/
#pragma once

#ifndef TUVOK_CONTEXT_ID_H
#define TUVOK_CONTEXT_ID_H

namespace tuvok {

/// Defines an interface by which we can query and compare context
/// information.  We use this in our GPU memory manager to localize
/// textures to a particular context, since textures cannot travel
/// between contexts easily.
///
/// You generally don't want to use this class directly.  Include
/// `Context.h', which will create a `CTContext' (compile-time context)
/// type, and use the CTContext type in your code.
///
/// To implement a new type of context, you must define the comparison
/// operators (of course) and a `Current' method which obtains the
/// currently active context and wraps it into your object.  Derive
/// your class from this one, templatized on your class.  Generally
/// you'll want a private variable to hold a context given or implied
/// via the constructors.  See the GL/ implementations for an example.
template<class Context>
class ContextID {
  public:
    static Context Current() {
      return Context::CurrentCtx();
    }

  protected:
    ContextID() {}

  private:
    ContextID& operator=(const ContextID &); ///< undefined
    ContextID(const ContextID &); ///< undefined.
    bool operator==(const ContextID&) const; ///< undefined
    bool operator!=(const ContextID&) const; ///< undefined
};

};

#endif // TUVOK_CONTEXT_ID_H
