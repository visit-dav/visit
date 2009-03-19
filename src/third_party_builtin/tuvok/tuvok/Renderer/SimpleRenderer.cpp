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
  \file    SimpleRenderer.cpp
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   SimpleRenderer is a dumbed-down renderer for dealing with a single
           volume (no LOD hierarchies).
*/
#include "SimpleRenderer.h"
#include "Controller/Controller.h"

SimpleRenderer::SimpleRenderer() :
  AbstrRenderer(&(Controller::Instance()), true, false, false)
{
}

AbstrRenderer::EViewMode SimpleRenderer::GetViewmode() {
  WARNING("This renderer only supports one view mode.");
  return AbstrRenderer::GetViewmode();
}
void SimpleRenderer::SetViewmode(EViewMode) {
  T_ERROR("This renderer only supports one view mode.");
}

bool SimpleRenderer::LoadDataset(const std::string&)
{
  T_ERROR("This renderer only accesses datasets already resident.");
  return false;
}

UINT64 SimpleRenderer::GetCurrentSubFrameCount() const
{
  T_ERROR("This renderer does not do scheduling.");
  return 0;
}
UINT32 SimpleRenderer::GetWorkingSubFrame() const
{
  T_ERROR("This renderer does not do scheduling.");
  return 0;
}
UINT32 SimpleRenderer::GetCurrentBrickCount() const
{
  T_ERROR("This renderer does not do scheduling.");
  return 0;
}
UINT32 SimpleRenderer::GetWorkingBrick() const
{
  T_ERROR("This renderer does not do scheduling.");
  return 0;
}

UINT32 SimpleRenderer::GetFrameProgress() const
{
  T_ERROR("This renderer does not do scheduling.");
  return 0;
}
UINT32 SimpleRenderer::GetSubFrameProgress() const
{
  T_ERROR("This renderer does not do scheduling.");
  return 0;
}

void SimpleRenderer::SetTimeSlice(UINT32)
{
  T_ERROR("This renderer does not do scheduling.");
}
void SimpleRenderer::SetPerfMeasures(UINT32, UINT32)
{
  T_ERROR("This renderer does not do scheduling.");
}

void SimpleRenderer::ScheduleRecompose()
{
  T_ERROR("Drawing is immediate for this renderer.");
}
