// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// .NAME vtkVisitFullFrameMapper2D - Allows setting full frame mode and scale.
// .SECTION Description
// vtkVisItFullFrameMapper2D is a vtkMapper2D that allows users to set the
// full frame mode and scale. How the full frame information is used is
// up to the inheriting classes.
//
// .SECTION Caveats
//
// .SECTION See Also
// vtkMapper2D

#ifndef __vtkVisItFullFrameMapper2D_h
#define __vtkVisItFullFrameMapper2D_h
#include <plotter_exports.h>

#include "vtkMapper2D.h"

// ***************************************************************************
//  Modifications:
// ***************************************************************************

class PLOTTER_API vtkVisItFullFrameMapper2D : 
  public vtkMapper2D
{
    public:
        static vtkVisItFullFrameMapper2D *New();
        vtkTypeMacro(vtkVisItFullFrameMapper2D, vtkMapper2D);
    
        void PrintSelf(ostream& os, vtkIndent indent) override;
    
        bool SetFullFrameScaling(bool, const double *);
        bool SetFullFrameScaling(const double *);
    
    protected:
        vtkVisItFullFrameMapper2D();
       ~vtkVisItFullFrameMapper2D();
      
        double FullFrameScaling[3];
        bool   UseFullFrameScaling;
};

#endif
