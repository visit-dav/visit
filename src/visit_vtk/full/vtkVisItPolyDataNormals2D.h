/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#ifndef __vtkVisItPolyDataNormals2D_h
#define __vtkVisItPolyDataNormals2D_h

#include <visit_vtk_exports.h>
#include <vtkPolyDataAlgorithm.h>

class vtkPolyData;
class vtkInformation;
class vtkInformationVector;

// ****************************************************************************
//  Class:  vtkVisItPolyDataNormals2D
//
//  Purpose:
//    Calculate cell or point centered normals.
//
//  Note: Derived from avtSurfaceNormalExpression version of function
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 10, 2014
//
// ****************************************************************************
class VISIT_VTK_API vtkVisItPolyDataNormals2D : public vtkPolyDataAlgorithm
{
  public:
    vtkTypeMacro(vtkVisItPolyDataNormals2D,vtkPolyDataAlgorithm);

    static vtkVisItPolyDataNormals2D *New();

    void SetNormalTypeToCell()      { ComputePointNormals = false; }
    void SetNormalTypeToPoint()     { ComputePointNormals = true;  }

  protected:
    vtkVisItPolyDataNormals2D();
    ~vtkVisItPolyDataNormals2D() {};

    // Usual data generation method
    int RequestData(vtkInformation*, vtkInformationVector**,
                     vtkInformationVector*);
    void ExecutePoint(vtkPolyData*, vtkPolyData*);
    void ExecuteCell(vtkPolyData *, vtkPolyData*);

    bool  ComputePointNormals;

  private:
    vtkVisItPolyDataNormals2D(const vtkVisItPolyDataNormals2D&);  // Not implemented.
    void operator=(const vtkVisItPolyDataNormals2D&);  // Not implemented.
};

#endif
