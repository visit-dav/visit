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

#ifndef VTK_CRACKS_CLIPPER_H
#define VTK_CRACKS_CLIPPER_H

#include <vtkVisItClipper.h>
#include <vtkImplicitFunction.h>

class vtkImplicitBoolean;


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
//
// ****************************************************************************

class vtkCracksClipper : public vtkVisItClipper
{
  public:
    vtkTypeRevisionMacro(vtkCracksClipper,vtkVisItClipper);

    static vtkCracksClipper *New();

    virtual void SetUpClipFunction(int);
    virtual void SetUseOppositePlane(bool val) { useOppositePlane = val;};

    vtkSetStringMacro(CrackDir);
    vtkSetStringMacro(CrackWidth);
    vtkSetStringMacro(CellCenters);

  protected:
    vtkCracksClipper();
    ~vtkCracksClipper();

  private:
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


