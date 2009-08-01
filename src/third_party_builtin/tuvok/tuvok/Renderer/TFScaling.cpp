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
  \file    TFScaling
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   Functions for scaling transfer functions.
*/

#include <cmath>
#include "TFScaling.h"
#include "IO/Dataset.h"
#include "IO/Metadata.h"
#include "IO/TransferFunction1D.h"

namespace tuvok {

float
scale_bit_width(const Metadata& md, bool downsample,
                const TransferFunction1D& tf)
{
  size_t max_value = (md.GetBitWidth() != 8 && downsample)
                      ? 65536 : tf.GetSize();
  UINT32 max_range = static_cast<UINT32>(1 << md.GetBitWidth());

  return (md.GetBitWidth() != 8 && downsample)
          ? 1.0f : static_cast<float>(max_range) /
                   static_cast<float>(max_value);
}

std::pair<float,float>
scale_bias_and_scale(const Metadata& md)
{
  std::pair<float,float> retval;

  // bias by the minimum value in the dataset.
  retval.first = -(md.GetRange().first);

  // scale by the full range of the data
  retval.second = fabsf(md.GetRange().first) +
                  fabsf(md.GetRange().second);
  return retval;
}

}; // tuvok namespace.
