/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#ifndef VTK_VISIT_CLIPPER_H
#define VTK_VISIT_CLIPPER_H

#include <visit_vtk_exports.h>
#include "vtkDataSetToUnstructuredGridFilter.h"

class vtkImplicitFunction;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class:  vtkVisItClipper
//
//  Purpose:
//    Clips a dataset using an implicit function.  This is a wholesale
//    replacement for the internal VTK clipper for structured, rectilinear,
//    and unstructured datasets with three topological dimensions and cell
//    types that are a part of the normal zoo.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 11, 2003
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 30 16:15:50 PST 2004
//    Added scalar array and scalar range for isovolume.
//
//    Jeremy Meredith, Mon Feb 16 19:08:16 PST 2004
//    Added PolyData support.
//
//    Jeremy Meredith, Wed May  5 13:06:14 PDT 2004
//    Renamed without the "3D" because I also made it support 2D.
//    Changed it to a single cutoff for scalars to make the math more robust.
//
//    Jeremy Meredith, Tue Aug 29 13:43:56 EDT 2006
//    Added support for forcing removal of entire cells instead of clipping.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added flag to use zero-crossings of the implicit function. 
//    Added flat to compute inside and out in single execute.
//
//    Hank Childs, Sat Sep 29 11:14:58 PDT 2007
//    Make change to argument of function (SetClipScalars), which is necessary
//    for non-traditional cell types (hex-20).  Also add support for data
//    types beyond floats.
//
// ****************************************************************************

class VISIT_VTK_API vtkVisItClipper
    : public vtkDataSetToUnstructuredGridFilter
{
  public:
    vtkTypeRevisionMacro(vtkVisItClipper,vtkDataSetToUnstructuredGridFilter);
    void PrintSelf(ostream& os, vtkIndent indent);

    static vtkVisItClipper *New();

    virtual void SetRemoveWholeCells(bool);
    virtual void SetClipFunction(vtkImplicitFunction*);
    virtual void SetClipScalars(vtkDataArray *, float);
    virtual void SetInsideOut(bool);
    virtual void SetComputeInsideAndOut(bool);
    virtual void SetUseZeroCrossings(bool);
    virtual vtkUnstructuredGrid *GetOtherOutput();

    void SetCellList(int *, int);
    virtual void SetUpClipFunction(int) { ; };


  protected:
    vtkVisItClipper();
    ~vtkVisItClipper();

    void Execute();
    void RectilinearGridExecute();
    void StructuredGridExecute();
    void UnstructuredGridExecute();
    void PolyDataExecute();
    void GeneralExecute();
    void ClipDataset(vtkDataSet *, vtkUnstructuredGrid *);

    int *CellList;
    int  CellListSize;
  private:
    bool   removeWholeCells;
    bool   insideOut;
    vtkImplicitFunction *clipFunction;
    bool   iOwnData;
    float *scalarArray;
    vtkDataArray *scalarArrayAsVTK;
    float  scalarCutoff;
    bool   scalarFlip;
    bool   useZeroCrossings;
    bool   computeInsideAndOut;

    vtkUnstructuredGrid *otherOutput;

    vtkVisItClipper(const vtkVisItClipper&);  // Not implemented.
    void operator=(const vtkVisItClipper&);  // Not implemented.
};


#endif


