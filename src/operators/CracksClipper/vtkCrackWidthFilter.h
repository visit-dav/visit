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

#ifndef VTK_CRACK_WIDTH_FILTER_H
#define VTK_CRACK_WIDTH_FILTER_H

#include <vtkDataSetToDataSetFilter.h>

class vtkCell;
class vtkMassProperties;
class vtkQuad;
class vtkSlicer;
class vtkTriangle;

// ****************************************************************************
//  Class:  vtkCrackWidthFilter
//
//  Purpose:
//    Calculates the width of cracks along given crack directions for
//    each cell, and stores each width in a cell array.  Also calculates
//    the cell centers.  
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    August 22, 2005 
//
//  Modifications:
//    Kathleen Bonnell, Wed Sep 13 07:42:59 PDT 2006
//    Remove individual cell intersection methods, use vtkCellIntersections 
//    class instead.
//
//    Kathleen Bonnell, Fri Oct 13 11:05:01 PDT 2006 
//    Removed use of vtkCellIntersections, added vtkMassProperties, vtkSlicer.
//    Changed args for CrackWidthForCell.
//
// ****************************************************************************

class vtkCrackWidthFilter : public vtkDataSetToDataSetFilter
{
  public:
    vtkTypeRevisionMacro(vtkCrackWidthFilter,vtkDataSetToDataSetFilter);

    static vtkCrackWidthFilter *New();

    vtkGetMacro(MaxCrack1Width, double);
    vtkGetMacro(MaxCrack2Width, double);
    vtkGetMacro(MaxCrack3Width, double);

    double GetMaxCrackWidth(int w);
    
    vtkSetStringMacro(Crack1Var);
    vtkSetStringMacro(Crack2Var);
    vtkSetStringMacro(Crack3Var);
    vtkSetStringMacro(StrainVar);

  protected:
    vtkCrackWidthFilter();
    ~vtkCrackWidthFilter();

    void Execute();

  private:
    vtkTriangle *triangle;
    vtkQuad *quad;
    double MaxCrack1Width;
    double MaxCrack2Width;
    double MaxCrack3Width;

    char *Crack1Var;
    char *Crack2Var;
    char *Crack3Var;
    char *StrainVar;

    vtkSlicer *Slicer;
    vtkMassProperties *MassProp;

    double  CrackWidthForCell(vtkCell *cell, int cellId, const double *center,
           const double delta, const double *dir,
           const double zvol, const double L1L2);

    vtkCrackWidthFilter(const vtkCrackWidthFilter&);  // Not implemented.
    void operator=(const vtkCrackWidthFilter&);  // Not implemented.

};



#endif


