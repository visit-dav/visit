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
  \file    Quantize.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   Quantization routines.
*/

#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <limits>
#include <boost/algorithm/minmax_element.hpp>
#include <boost/cstdint.hpp>
#include "IOManager.h"  // for the size defines
#include "Basics/LargeRAWFile.h"

/// Progress policies.  Must implement a constructor and `notify' method which
/// both take a `T'.  The constructor is given the max value; the notify method
/// is given the current value.
///    NullProgress -- nothing, use when you don't care.
///    TuvokProgress -- forward progress info to Tuvok debug logs.
///@{
template <typename T>
struct NullProgress {
  NullProgress(T) {};
  static void notify(T) {}
};
template <typename T>
struct TuvokProgress {
  TuvokProgress(T total) : tMax(total) {}
  void notify(T current) const {
    MESSAGE("Computing value range (%5.3f percent complete).",
            static_cast<double>(current) / static_cast<double>(tMax)*100.0);
  }
  private: T tMax;
};
///@}

/// (raw) Data source policies.  Must implement:
///   constructor: takes a filename
///   size(): returns the number of elements in the file.
///   read(data, b): reads `b' bytes in to `data'.  Returns number of elems
///                  actually read.
/// ios_data_src -- data source for C++ iostreams.
/// raw_data_src -- data source for Basics' LargeRAWFile.
///@{
template <typename T>
struct ios_data_src {
  ios_data_src(const char *fn) : ifs(fn, std::ios::binary), filename(fn) {
    if(!ifs.is_open()) {
      throw std::runtime_error(__FILE__);
    }
  }
  ios_data_src(const ios_data_src<T>& ios) : ifs(ios.filename) { }
  ~ios_data_src() { ifs.close(); }

  boost::uint64_t size() {
    std::streampos cur = ifs.tellg();
    ifs.seekg(0, std::ios::end);
    boost::uint64_t retval = ifs.tellg();
    ifs.seekg(cur, std::ios::beg);
    return retval/sizeof(T);
  }
  size_t read(unsigned char *data, size_t max_bytes) {
    ifs.read(reinterpret_cast<char*>(data), std::streamsize(max_bytes));
    return ifs.gcount()/sizeof(T);
  }
  private:
    std::ifstream ifs;
    const char *filename;
};

template <typename T>
struct raw_data_src {
  raw_data_src(const char *fn) : raw(fn) {
    raw.Open(false);
    if(!raw.IsOpen()) {
      throw std::runtime_error(__FILE__);
    }
  }
  raw_data_src(const raw_data_src<T>& r) : raw(r.raw.GetFilename()) {}
  ~raw_data_src() { raw.Close(); }

  boost::uint64_t size() { return raw.GetCurrentSize() / sizeof(T); }
  size_t read(unsigned char *data, size_t max_bytes) {
    return raw.ReadRAW(data, max_bytes)/sizeof(T);
  }
  private: LargeRAWFile raw;
};
///@}

/// Computes the minimum and maximum of a conceptually one dimensional dataset.
/// Takes policies tell it how to access data && notify external entities of
/// progress.
/// @todo shouldn't hardcode INCORESIZE in here.
template <typename T, template <typename T> class DataSrc, class Progress>
std::pair<T,T> io_minmax(DataSrc<T> ds, const Progress& progress)
{
  std::vector<T> data(INCORESIZE);
  boost::uint64_t iPos = 0;
  boost::uint64_t iSize = ds.size();

  std::pair<T,T> t_minmax(std::numeric_limits<T>::max(),
                          -(std::numeric_limits<T>::max()-1));
  while(iPos < iSize) {
    size_t n_records = ds.read((unsigned char*)(&(data.at(0))),
                               INCORESIZE*sizeof(T));
    data.resize(n_records);
    assert(n_records > 0);
    typedef typename std::vector<T>::const_iterator iterator;
    std::pair<iterator,iterator> cur_mm = boost::minmax_element(data.begin(),
                                                                data.end());
    t_minmax.first = std::min(t_minmax.first, *cur_mm.first);
    t_minmax.second = std::max(t_minmax.second, *cur_mm.second);

    iPos += boost::uint64_t(n_records*sizeof(T));

    progress.notify(iPos);
  }
  return t_minmax;
}

template <typename T,
          template <typename T> class DataSrc,
          template <typename T> class DataSink,
          class Progress>
void quantize(DataSrc<T> dsrc, DataSink<T> dsink, const Progress& progress,
              T qMin, T qMax)
{
  T range = qMax - qMin;
  std::vector<T> data(INCORESIZE);

  boost::uint64_t cur_pos = 0;
  boost::uint64_t size = dsrc.size();

  std::pair<T,T> minmax = io_minmax(dsrc, progress);

  while(cur_pos < size) {
    size_t n_records = dsrc.read((unsigned char*)(&(data.at(0))),
                                 INCORESIZE*sizeof(T));
    if(n_records == 0) { break; }
  }
}
