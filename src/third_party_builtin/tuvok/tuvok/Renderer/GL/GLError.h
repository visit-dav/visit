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
  \file    GLError.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
*/

#pragma once

#ifndef TUVOK_GL_ERROR_H
#define TUVOK_GL_ERROR_H

#include <sstream>
#include <stdexcept>

/// Defines OpenGL errors that we'll throw, and GL error detection code.

namespace tuvok {

/// Thrown when allocating an OpenGL resource fails due to out of memory
/// condition.
class OutOfMemory : virtual public std::runtime_error {
  public:
    explicit OutOfMemory(const char* e,
                         const char *where=NULL, size_t ln=0)
      : std::runtime_error(e), error(e), location(where), line(ln) { }
    virtual ~OutOfMemory() throw() { }

    virtual const char *what() const throw() { return this->error; }
    const char *where() const { return this->location; }
    size_t lineno() const { return this->line; }

  private:
    const char *error;
    const char *location;
    size_t line;
};

/// Intended to be thrown, e.g.:
///    throw OUT_OF_MEM("OpenGL 3d texture");
/// It automatically sets location information.
#define OUT_OF_MEMORY(s) tuvok::OutOfMemory(s, __FILE__, __LINE__)

/// Generic OpenGL error.
class GLError : virtual public std::runtime_error {
  public:
    explicit GLError(int glerr,
                     const char *where=NULL, size_t ln=0) :
      std::runtime_error("OpenGL error"), glerrno(glerr), location(where),
      line(ln) { }
    virtual ~GLError() throw() { }

    virtual const char *what() const throw() { return "OpenGL error"; }
    int errno() const { return this->glerrno; }
    const char *where() const { return this->location; }
    size_t lineno() const { return this->line; }

  private:
    int glerrno;
    const char *location;
    size_t line;
};

/// Intended to be thrown, e.g.:
///    throw GL_ERROR(42);
/// It automatically sets location information.
#define GL_ERROR(e) tuvok::GLError(e, __FILE__, __LINE__)

}; // namespace

#endif // TUVOK_GL_ERROR_H
