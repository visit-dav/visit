/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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

#ifndef VTK_CRACKS_CLIPPER_H
#define VTK_CRACKS_CLIPPER_H

#include <vtkImplicitFunction.h>
#include <vtkDataSetToUnstructuredGridFilter.h>

class vtkImplicitBoolean;
class vtkUnstructuredGrid;

class AlwaysNegative : public vtkImplicitFunction
{
public:
  vtkTypeMacro(AlwaysNegative, vtkImplicitFunction);
  static AlwaysNegative *New();

  virtual double EvaluateFunction(double x[3])
      { return -1.; }
 virtual void EvaluateGradient(double x[3], double g[3]) 
      {g[0] = g[1] = g[2] = 1.; }; 

  void SetReturnNeg(bool val) { returnNeg = val; }

protected:
  AlwaysNegative();
  ~AlwaysNegative();

private:
  bool returnNeg;
  AlwaysNegative(const AlwaysNegative&);  // Not implemented.
  void operator=(const AlwaysNegative&);  // Not implemented.
};


// ****************************************************************************
//  Class:  vtkCracksClipper
//
//  Purpose:
//    Clips a dataset using an implicit function, defined on a per-cell
//    basis based upon crack-direction vectors and strain variables.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    August 22, 2005 
//
//  Modifications:
//    Kathleen Bonnell, Thu Sep 23 13:28:57 MST 2010
//    Changed inheritance to vtkDataSetToUnstructuredGridFilter and added all
//    functionality from 1.12 version of vtkVisItClipper class.  Changes made
//    to vtkVisItClipper for 2.0 don't meet needs of CracksClipper, which 
//    applies a different Clip Function to every cell. 
//
// ****************************************************************************

class vtkCracksClipper
    : public vtkDataSetToUnstructuredGridFilter
{
  public:
    vtkTypeRevisionMacro(vtkCracksClipper,vtkDataSetToUnstructuredGridFilter);
    void PrintSelf(ostream& os, vtkIndent indent);

    static vtkCracksClipper *New();

    virtual void SetRemoveWholeCells(bool);
    virtual void SetClipFunction(vtkImplicitFunction*);
    virtual void SetClipScalars(vtkDataArray *, float);
    virtual void SetInsideOut(bool);
    virtual void SetComputeInsideAndOut(bool);
    virtual void SetUseZeroCrossings(bool);
    virtual vtkUnstructuredGrid *GetOtherOutput();

    void SetCellList(int *, int);
    void SetUpClipFunction(int);
    void SetUseOppositePlane(bool val) { useOppositePlane = val;};

    vtkSetStringMacro(CrackDir);
    vtkSetStringMacro(CrackWidth);
    vtkSetStringMacro(CellCenters);

  protected:
    vtkCracksClipper();
    ~vtkCracksClipper();

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
    vtkCracksClipper(const vtkCracksClipper&);  // Not implemented.
    void operator=(const vtkCracksClipper&);  // Not implemented.

    vtkImplicitBoolean *cf;
    AlwaysNegative *emptyFunc;
    bool useOppositePlane;

    char *CrackDir;
    char *CrackWidth;
    char *CellCenters;
};



#endif


