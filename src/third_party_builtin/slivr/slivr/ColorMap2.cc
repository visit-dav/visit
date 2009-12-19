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
//    File   : ColorMap2.cc
//    Author : Milan Ikits
//    Date   : Mon Jul  5 18:33:29 2004

#include <slivr/ColorMap2.h>
#include <iostream>

namespace SLIVR {
using namespace std;

ColorMap2::ColorMap2() : 
  updating_(false),
  widgets_(0),
  selected_(-1),
  value_range_(0.0, -1.0)
{}

ColorMap2::ColorMap2(const ColorMap2 &copy) : 
  updating_(copy.updating_),
  selected_(copy.selected_),
  value_range_(copy.value_range_)
{
  // copy in the new set.
  deep_copy_widgets(copy.widgets_, widgets_);
}

ColorMap2::ColorMap2(const vector<CM2Widget*>& widgets,
                     bool updating,
                     bool selected,
                     pair<float, float> value_range) : 
  updating_(updating),
  selected_(selected),
  value_range_(value_range)
{
  // copy in the new set.
  deep_copy_widgets(widgets, widgets_);
}

ColorMap2::~ColorMap2()
{
  // delete the widgets.
  for (size_t i = 0; i < widgets_.size(); ++i) {
    delete widgets_[i];
  }
  widgets_.clear();
}

} // End namespace SLIVR
