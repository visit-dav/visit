//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2008 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
//    File   : ColorMap2.h
//    Author : Milan Ikits
//    Date   : Mon Jul  5 18:33:12 2004

#ifndef SLIVR_ColorMap2_h
#define SLIVR_ColorMap2_h

#include <vector>
#include <utility>
#include <slivr/share.h>
#include <slivr/CM2Widget.h>

namespace SLIVR {

class CM2Widget;
using std::vector;
using std::pair;

class SLIVRSHARE ColorMap2 
{
public:
  ColorMap2();
  ColorMap2(const ColorMap2 &copy);
  ColorMap2(const vector<CM2Widget*>& widgets,
	    bool updating, 
	    bool selected,
	    pair<float,float> value_range );
  virtual ~ColorMap2();

  vector<CM2Widget*> &		        widgets() { return widgets_; }
  bool &				updating() { return updating_; }
  int &					selected() { return selected_; }
  pair<float, float> &			value_range() { return value_range_; }

protected:
  bool					updating_;
  vector<CM2Widget*>		        widgets_;
  int					selected_;
  pair<float, float>			value_range_;
};

inline
void deep_copy_widgets(const vector<SLIVR::CM2Widget*> &src, 
		       vector<SLIVR::CM2Widget*> &dst)
{
  // copy in the new set.
  dst.resize(src.size());
  for (size_t i = 0; i < src.size(); ++i) {
    dst[i] = src[i]->duplicate();
  }
}

} // End namespace SLIVR

#endif // ColorMap2_h
