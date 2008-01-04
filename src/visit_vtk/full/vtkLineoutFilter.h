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

//=========================================================================
// .NAME vtkLineoutFilter - 
// .SECTION Description
// vtkLineoutFilter is a filter object that applies a probe filter to
// the input.  The probe filter output is transformed into x-y pairs
// (vertices), with x representing the distance along the probe-line
// and y the interpolated scalar value at that point. 
//
// Points determined as 'invalid' by the probe filter
// are not included in the output.
//
//

#ifndef __vtkLineoutFilter_h
#define __vtkLineoutFilter_h

#include <vtkDataSetToPolyDataFilter.h>
#include <vtkSetGet.h>
#include <visit_vtk_exports.h>

class vtkCellDataToPointData;
class vtkLineSource;
class vtkProbeFilter;

class VISIT_VTK_API vtkLineoutFilter : public vtkDataSetToPolyDataFilter
{
public:
  static vtkLineoutFilter *New();
  vtkTypeMacro(vtkLineoutFilter,vtkDataSetToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the endpoints of the line used for probing. 
  vtkSetVector3Macro(Point1, double); 
  vtkGetVectorMacro(Point1, double, 3); 

  vtkSetVector3Macro(Point2, double); 
  vtkGetVectorMacro(Point2, double, 3); 

  vtkSetMacro(NumberOfSamplePoints, int);
  vtkGetMacro(NumberOfSamplePoints, int);


protected:
  vtkLineoutFilter();
  ~vtkLineoutFilter() ;

  vtkCellDataToPointData *cd2pd;
  vtkLineSource          *LineSource;
  vtkProbeFilter         *Probe;

  void Execute();

private:
  double          Point1[3];
  double          Point2[3];
  int             NumberOfSamplePoints;

  vtkLineoutFilter(const vtkLineoutFilter&);
  void operator=(const vtkLineoutFilter&);
};

#endif


