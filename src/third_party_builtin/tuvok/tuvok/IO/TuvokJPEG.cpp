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
  \file    TuvokJPEG.cpp
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   Tuvok wrapper for JPEG loading code.
*/
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>
#include "3rdParty/jpeglib/jpeglib.h"
#include "TuvokJPEG.h"
#include "Controller/Controller.h"

/// To read from something with a JPEG embedded, like a DICOM, we need to be
/// able to read from an arbitrary buffer.  We support a typical file
/// interface, but we also need to support the former use case.  Thus the file
/// interface works by opening the file, slurping in the entire buffer, and
/// then reading the data as if it had always been in memory.  This unifies our
/// code paths, but requires more memory than is strictly necessary.

namespace tuvok {

// To do JPEG IO which isn't from a file, one has to set up a special object
// with a particular set of methods.  We'll reuse most of the methods from
// what libjpeg thinks is a stdio src, which works because we've pre-populated
// the buffer.  Thus libjpeg will never make a request to the source manager to
// read more data.
typedef struct {
  jpeg_source_mgr pub; /* public fields */
  JOCTET *buffer;      /* start of buffer */
} incore_src_mgr;

// Well, mostly.  Just to be sure, we override the `ask for more data' method
// to ensure it'll never be called.  We'll also install our own error handler,
// because libjpeg's default terminates the process.
static boolean fill_input_buffer(j_decompress_ptr);
static void jpg_error(j_common_ptr);

static void fill_buffer_from_file(std::vector<char>& buf, const char *fn,
                                  std::streamoff offset);

/// To avoid including jpeglib in the header, we create an opaque pointer to
/// the `jpeg implementation'.  This is what we'll actually end up pointing to.
struct j_implementation : public JPEG::j_impl {
  // Constructor just configures the objects we'll use later.
  j_implementation() : started(false) {
    struct jpeg_decompress_struct jpg;
    jpg.err = jpeg_std_error(&(this->jerr));
    jpg.err->error_exit = jpg_error;
    jpeg_create_decompress(&jpg);
    jpeg_stdio_src(&jpg, NULL);

    this->jinfo = jpg;
  };

  // Expects to get the entire JPEG buffer, but only reads the header.
  // After calling this, it is safe to query the JPEG metadata from
  // this->jinfo.
  // Copies the data from the argument.
  void set_data(const std::vector<char>& mem) {
    this->data = mem;
    this->jinfo.src->bytes_in_buffer = this->data.size();
    this->jinfo.src->next_input_byte = &(this->data.at(0));
    if(jpeg_read_header(&(this->jinfo), TRUE) != JPEG_HEADER_OK) {
      T_ERROR("Could not read JPEG header, bailing...");
      return;
    }
    jpeg_start_decompress(&(this->jinfo));
    started = true;
  }

  virtual ~j_implementation() {
    // If the client didn't transfer any actual data (i.e. only read
    // metadata), then this `finish' causes an `Application transferred too few
    // scanlines' warning.  Yet we've called start_decompress, so we must call
    // finish.  Just live with the warning, or fix the issue in libjpeg itself.
    if(started) {
      jpeg_finish_decompress(&this->jinfo);
      jpeg_destroy_decompress(&this->jinfo);
    }
  }
  std::vector<char> data;       ///< hunk of memory we'll read the jpeg from.
  bool started;                 ///< whether we've started decompressing
  jpeg_error_mgr jerr;          ///< how jpeg will report errors
  jpeg_decompress_struct jinfo; ///< main interface for decompression
};

JPEG::JPEG(const std::string &fn, std::streamoff offset) :
  w(0), h(0), bpp(0), jpeg_impl(new j_implementation)
{
  fill_buffer_from_file(this->buffer, fn.c_str(), offset);
  this->initialize();
}

JPEG::JPEG(const std::vector<char>& buf) :
  w(0), h(0), bpp(0), jpeg_impl(new j_implementation)
{
  this->buffer = buf;
  this->initialize();
}

JPEG::~JPEG()
{
  delete this->jpeg_impl;
}

bool JPEG::valid() const
{
  return this->w != 0 &&
         this->h != 0 &&
         this->bpp != 0;
}

size_t JPEG::width() const { return this->w; }
size_t JPEG::height() const { return this->h; }
size_t JPEG::components() const {return this->bpp; }
size_t JPEG::size() const { return this->w * this->h * this->bpp; }

// Decode the JPEG data and give back the raw array.
const char* JPEG::data()
{
  // If the JPEG isn't valid, we can't load anything from it; just bail.
  if(!this->valid()) { return NULL; }

  // Need a buffer for the image data.
  this->buffer.resize(this->size());

  j_implementation *jimpl = dynamic_cast<j_implementation*>(this->jpeg_impl);

  {
    void *jbuffer = NULL;
    const size_t row_sz = this->w * this->bpp;
    jbuffer = (*jimpl->jinfo.mem->alloc_sarray)(
                 (j_common_ptr) &(jimpl->jinfo),
                 JPOOL_IMAGE,
                 JDIMENSION(row_sz),
                 1
              );

    assert(this->h == jimpl->jinfo.output_height);
    assert(this->height() == jimpl->jinfo.output_height);
    assert(this->width() > 0);
    assert(this->components() > 0);
    char *data = &(this->buffer.at(0));

    while(jimpl->jinfo.output_scanline < jimpl->jinfo.output_height) {
      jpeg_read_scanlines(&(jimpl->jinfo), (JSAMPLE**)&jbuffer, 1);
      // The -1 is because jpeg_read_scanlines implicitly incremented the
      // output_scanline field, putting the current scanline after the call 1
      // in front of the scanline we want to copy.
      assert(jimpl->jinfo.output_scanline > 0);
      std::memcpy(data + ((jimpl->jinfo.output_scanline-1)*row_sz),
                  jbuffer, row_sz);
    }
  }
  return &this->buffer.at(0);
}

void JPEG::initialize()
{
  j_implementation *jimpl = dynamic_cast<j_implementation*>(this->jpeg_impl);

  jimpl->jinfo.src->fill_input_buffer = fill_input_buffer;
  jimpl->set_data(this->buffer);

  this->w = jimpl->jinfo.output_width;
  this->h = jimpl->jinfo.output_height;
  this->bpp = jimpl->jinfo.output_components;
}

/// Overly complex, too much so to be worth explaining.  Basically JPEG will
/// call this when it runs out of data to process.  In our case this should
/// never happen, because we read the whole JPEG before giving it to the
/// library -- all the data are already resident.
boolean fill_input_buffer(j_decompress_ptr)
{
  T_ERROR("unexpected fill_input_buffer");
  return TRUE;
}

// Just output the message and return.  We override this method because libjpeg
// would kill our process with its default implementation.
static void
jpg_error(j_common_ptr jinfo)
{
  // Unfortunately it seems pretty difficult to pull the actual error string
  // out of the library.  We'd like this to go to our debug logs, but it'll end
  // up on stderr.
  (*jinfo->err->output_message)(jinfo);
}

/// Exactly what it says; fill the contents of the buffer with the data in the
/// given filename.  Not a great idea to do this with large files.
static void
fill_buffer_from_file(std::vector<char>& buf, const char *fn,
                      std::streamoff offset)
{
  std::ifstream ifs(fn, std::ifstream::binary);

  // get filesize.
  ifs.seekg(0, std::ios::end);
  std::ifstream::pos_type file_size = ifs.tellg() - offset;
  ifs.seekg(offset, std::ios::beg);

  MESSAGE("Reading %d byte file.", static_cast<unsigned int>(file_size));

  // resize our buffer to be big enough for the file
  try {
    buf.resize(file_size);
  } catch(std::bad_alloc &ba) {
    T_ERROR("Could not allocate JPEG buffer (%s)",ba.what());
    WARNING("Retrying with smaller buffer...");
    file_size = 256*256*256;
    buf.resize(file_size);
  }
  ifs.read(&buf.at(0), file_size);
  assert(ifs.gcount() == file_size);
}

}; // namespace tuvok
