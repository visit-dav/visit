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
//    File   : ShaderProgramARB.h
//    Author : Milan Ikits
//    Date   : Wed Jul  7 23:20:59 2004

#ifndef ShaderProgramARB_h 
#define ShaderProgramARB_h

#include <string>
#include <iostream>

namespace SLIVR {

#include "slivr_share.h"

class SLIVRSHARE ShaderProgramARB
{
  public:
    ShaderProgramARB(const std::string& program);
    ~ShaderProgramARB();
    
    bool create(std::string& error);
    bool valid();
    void destroy();

    void bind();
    void release();

    void setLocalParam(int, float, float, float, float);

    // Call init_shaders_supported before shaders_supported queries!
    static bool init_shaders_supported(std::string& error);

    static bool shaders_supported();
    static bool initialized();
    static int max_texture_size_1();
    static int max_texture_size_4();
    static bool texture_non_power_of_two();
    static const int MAX_SHADER_UNIFORMS = 4;
    
    // OBSOLETE CALLS WITH BAD ERROR CHECKING
    static bool init_shaders_supported() 
    { 
      std::string error; 
      bool ret = init_shaders_supported(error); 
      if (!ret) std::cerr << error;
      return (ret);
    }    
    
    bool create()
    {
      std::string error;
      bool ret = create(error);
      if (!ret) std::cerr << error;
      // Oddly enough true meant that the creation failed
      return (!ret);
    }
    
  protected:
    unsigned int type_;
    unsigned int id_;
    std::string  program_;

    static bool init_;
    static bool supported_;
    static bool non_2_textures_;
    static int  max_texture_size_1_;
    static int  max_texture_size_4_;
  };

class SLIVRSHARE VertexProgramARB : public ShaderProgramARB
{
public:
  VertexProgramARB(const std::string& program);
  ~VertexProgramARB();
};

class SLIVRSHARE FragmentProgramARB : public ShaderProgramARB
{
public:
  FragmentProgramARB(const std::string& program);
  ~FragmentProgramARB();
};

} // end namespace SLIVR

#endif // ShaderProgramARB_h
