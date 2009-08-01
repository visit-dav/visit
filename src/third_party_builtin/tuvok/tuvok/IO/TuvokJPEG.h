/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
  \file    TuvokJPEG.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   Tuvok wrapper for JPEG loading code.
*/
#pragma once

#ifndef TUVOK_JPEG_H
#define TUVOK_JPEG_H

#include <iosfwd>
#include <string>
#include <vector>

namespace tuvok {

/// Simple wrapper for reading a JPEG using libjpeg.
class JPEG {
  public:
    /// Loads a JPEG from a file, starting at the given offset.
    JPEG(const std::string &, std::streamoff offset=0);

    /// Loads a JPEG from an in-memory buffer.
    JPEG(const std::vector<char>& buf);
    ~JPEG();

    /// Query for validity of the given JPEG.  If we can't decode the header,
    /// this returns false.
    /// Note that if a JPEG stream is corrupt in its payload but not its
    /// header, we'll say it's valid here but won't actually be able to read
    /// it.
    bool valid() const;

    /// Convenient accessors for image metadata.
    ///@{
    size_t width() const;
    size_t height() const;
    size_t components() const;
    /// Convenience; simply width * height * components.
    size_t size() const;
    ///@}

    /// Reads the underlying JPEG data and returns the data as a raw array.
    /// The given pointer is valid for the lifetime of this object.
    const char* data();

  public:
    /// for avoiding a jpeglib.h dependence.
    struct j_impl { virtual ~j_impl() {} };

  private:
    /// No reason one couldn't define this, but it'd require setting the buffer
    /// after construction.  Since we don't have that needed companion method
    /// currently, this doesn't make any sense.
    JPEG();

    /// Sets up jpeg_impl.  Reads header information.
    void initialize();

  private:
    size_t w, h;
    size_t bpp;
    std::vector<char> buffer;
    j_impl *jpeg_impl;
};

};

#endif // TUVOK_JPEG_H
