/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// .NAME vtkRectilinearGridFacelistFilter - get facelists for a rectilinear grid
// .SECTION Description
// vtkRectilinearGridFacelistFilter is a filter that extracts facelists from a
// rectilinear grid.

// .SECTION See Also
// vtkGeometryFilter

#ifndef __vtkRectilinearGridFacelistFilter_h
#define __vtkRectilinearGridFacelistFilter_h
#include <visit_vtk_exports.h>

#include "vtkRectilinearGridToPolyDataFilter.h"
class     vtkIdTypeArray;
#include <vector>

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Sun Nov  9 12:37:15 PST 2003
//    Added separate consolidation routines that work with and without ghost
//    zones.
//
//    Hank Childs, Tue Jan 24 10:11:22 PST 2006
//    Add support for ghost nodes.
//
// ****************************************************************************

class VISIT_VTK_API vtkRectilinearGridFacelistFilter : public vtkRectilinearGridToPolyDataFilter
{
public:
  vtkTypeMacro(vtkRectilinearGridFacelistFilter,vtkRectilinearGridToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create a vtkRectilinearGridFacelistFilter.
  static vtkRectilinearGridFacelistFilter *New();

  // Description:
  // Set/Get ForceFaceConsolidation
  vtkSetMacro(ForceFaceConsolidation, int);
  vtkGetMacro(ForceFaceConsolidation, int);

protected:
  vtkRectilinearGridFacelistFilter();
  ~vtkRectilinearGridFacelistFilter() {};

  void Execute();

  int  ForceFaceConsolidation;

private:
  vtkRectilinearGridFacelistFilter(const vtkRectilinearGridFacelistFilter&);
  void operator=(const vtkRectilinearGridFacelistFilter&);

  void ConsolidateFacesWithoutGhostZones(void);
  vtkPolyData *ConsolidateFacesWithGhostZones(vtkPolyData *, vtkIdTypeArray *,
                       std::vector<int>&, std::vector<int>&,std::vector<int>&);
};

#endif


