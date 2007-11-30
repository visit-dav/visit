/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#ifndef AVT_OPEN_GL_SPREADSHEET_TRACER_RENDERER_H
#define AVT_OPEN_GL_SPREADSHEET_TRACER_RENDERER_H
#include <avtSpreadsheetTraceRenderer.h>

class vtkDataArray;
class vtkDataSet;
class vtkRectilinearGrid;
class vtkStructuredGrid;
class vtkCell;

// ****************************************************************************
// Class: avtOpenGLSpreadsheetTraceRenderer
//
// Purpose:
//   OpenGL version of the spreadsheet highlight renderer.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 09:17:47 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

class avtOpenGLSpreadsheetTraceRenderer : public avtSpreadsheetTraceRenderer
{
public:
    avtOpenGLSpreadsheetTraceRenderer();
    virtual ~avtOpenGLSpreadsheetTraceRenderer();

    virtual void Render(vtkDataSet *, vtkDataArray *, 
                        const SpreadsheetAttributes &, const double *);
private:
    void DrawRectilinearGrid(vtkRectilinearGrid *, vtkDataArray *, 
                             const SpreadsheetAttributes &, const double *);
    void DrawStructuredGrid(vtkStructuredGrid *, vtkDataArray *, 
                            const SpreadsheetAttributes &, const double *);
    void DrawBoundingBox(vtkDataArray *bounds, const double *);
    void Draw2DCell(vtkCell *cell, const double *fgColor);
    void Draw3DCell(vtkCell *cell, const double *fgColor);
};

#endif
