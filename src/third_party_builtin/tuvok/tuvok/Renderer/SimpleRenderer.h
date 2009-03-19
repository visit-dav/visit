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
  \file    SimpleRenderer.h
  \author  Tom Fogal
           SCI Institute
           University of Utah
  \brief   SimpleRenderer is a dumbed-down renderer for dealing with a single
           volume (no LOD hierarchies).
*/


#pragma once

#ifndef TUVOK_SIMPLE_RENDERER_H
#define TUVOK_SIMPLE_RENDERER_H

#include "AbstrRenderer.h"

class SimpleRenderer : public AbstrRenderer {
  public:
    SimpleRenderer();

    EViewMode GetViewmode();
    virtual void SetViewmode(EViewMode eViewMode);

    /// does nothing; makes no sense for this renderer.
    virtual bool LoadDataset(const std::string&);

    /// @return true; always need to redraw with this renderer.
    virtual bool CheckForRedraw() { return true; }

    /** Deallocates GPU memory allocated during the rendering process. */
    virtual void Cleanup() = 0;

    /// ignore the message, this renderer redraws on paint, always, and never
    /// any other time.
    ///@{
    virtual void Changed1DTrans() { }
    virtual void Changed2DTrans() { }
    ///@}

    /// scheduling routines
    /// These are disabled for this renderer.
    ///@{
    UINT64 GetCurrentSubFrameCount() const;
    UINT32 GetWorkingSubFrame() const;
    UINT32 GetCurrentBrickCount() const;
    UINT32 GetWorkingBrick() const;

    UINT32 GetFrameProgress() const;
    UINT32 GetSubFrameProgress() const;

    void SetTimeSlice(UINT32 iMSecs);
    void SetPerfMeasures(UINT32 iMinFramerate, UINT32 iStartDelay);
    ///@}

  protected:
    virtual void        ScheduleRecompose();
    virtual void        ClearDepthBuffer() = 0;
    virtual void        ClearColorBuffer() = 0;
};

#endif // TUVOK_SIMPLE_RENDERER_H
