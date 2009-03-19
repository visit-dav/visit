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
  \file    GeometryGenerator.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    Januar 2009
*/

#pragma once

#ifndef GEOMETRYGENERATOR_H
#define GEOMETRYGENERATOR_H

#include "../StdTuvokDefines.h"
#include <vector>
#include "../Basics/Vectors.h"

class PosNormalVertex {
public:
  PosNormalVertex(const FLOATVECTOR3& vPos = FLOATVECTOR3(0,0,0), const FLOATVECTOR3& vNormal = FLOATVECTOR3(1,0,0)) :
      m_vPos(vPos), m_vNormal(vNormal) {}

  FLOATVECTOR3 m_vPos;
  FLOATVECTOR3 m_vNormal;
};

class Triangle {
public:
  Triangle(const PosNormalVertex& a, const PosNormalVertex& b, const PosNormalVertex& c) {
    m_vertices[0] = a;
    m_vertices[1] = b;
    m_vertices[2] = c;
  }
  PosNormalVertex m_vertices[3];;
};

/** \class GeometryGenerator */
class GeometryGenerator {
public:
  static std::vector<Triangle> GenArrow(float  fOverallLength,
                                        float  fShaftToHeadRatio,
                                        float  fShaftRadius,
                                        float  fHeadRadius,
                                        UINT32 iSegments);

};

#endif // GEOMETRYGENERATOR_H
