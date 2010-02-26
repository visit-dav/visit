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

#ifndef VTK_VISIT_SPLITTER_H
#define VTK_VISIT_SPLITTER_H

#include <visit_vtk_exports.h>
#include <FixedLengthBitField.h>
#include <vector>
#include "vtkDataSetToUnstructuredGridFilter.h"

class vtkImplicitFunction;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class:  vtkVisItSplitter
//
//  Purpose:
//    Splits a dataset using an implicit function or a scalars variable,
//    tagging output cells as it splits, and later allowing extraction
//    of various region sets as whole data sets.
//
//  Note: Copied largely from vtkVisItClipper
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 24, 2010
//
//  Modifications:
//
// ****************************************************************************

class VISIT_VTK_API vtkVisItSplitter
    : public vtkDataSetToUnstructuredGridFilter
{
  public:
    vtkTypeRevisionMacro(vtkVisItSplitter,vtkDataSetToUnstructuredGridFilter);
    void PrintSelf(ostream& os, vtkIndent indent);

    static vtkVisItSplitter *New();

    virtual void SetRemoveWholeCells(bool);
    virtual void SetClipFunction(vtkImplicitFunction*);
    virtual void SetClipScalars(vtkDataArray *, float);
    virtual void SetInsideOut(bool);
    virtual void SetUseZeroCrossings(bool);
    virtual void SetOldTagBitField(std::vector<FixedLengthBitField<16> >*);
    virtual void SetNewTagBitField(std::vector<FixedLengthBitField<16> > *);
    virtual void SetNewTagBit(int);

    void SetCellList(int *, int);
    virtual void SetUpClipFunction(int) { ; };


  protected:
    vtkVisItSplitter();
    ~vtkVisItSplitter();

    void Execute();

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
    std::vector<FixedLengthBitField<16> > *oldTags;
    std::vector<FixedLengthBitField<16> > *newTags;
    int    newTagBit;

    vtkVisItSplitter(const vtkVisItSplitter&);  // Not implemented.
    void operator=(const vtkVisItSplitter&);  // Not implemented.
};


#endif


