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

// ************************************************************************* //
//                              avtTransform.h                               //
// ************************************************************************* //

#ifndef AVT_TRANSFORM_H
#define AVT_TRANSFORM_H

#include <filters_exports.h>

#include <avtDataTreeIterator.h>

class     vtkMatrix4x4;
class     vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtTransform
//
//  Purpose:
//      Does any 4x4 matrix transformation.  Derived types specify the matrix.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001
//    Changed ExecuteDomain to ExecuteData.
//
//    Hank Childs, Wed Jun  6 09:12:58 PDT 2001
//    Removed CalcDomainList.
//
//    Jeremy Meredith, Mon Sep 24 14:21:13 PDT 2001
//    Added UpdateDataObjectInfo.  Made inheritance virtual.
//
//    Hank Childs, Mon Jan 14 09:01:18 PST 2002
//    Added data member for transform filter to fix memory leak.
//
//    Hank Childs, Mon Aug  5 10:29:22 PDT 2002
//    Added individual routines to transform rectilinear grids based on what
//    type of transformation is done.
//
//    Hank Childs, Tue Sep 10 15:21:28 PDT 2002
//    Perform all memory management at the domain (ExecuteData) level.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Kathleen Bonnell, Fri Mar 28 14:33:55 PDT 2008 
//    Added TransformData method.
//
//    Hank Childs, Tue Sep 22 20:26:35 PDT 2009
//    Add a method for transforming vectors ... we need to not transform
//    vectors when volume rendering.
//
// ****************************************************************************

class AVTFILTERS_API avtTransform : public virtual avtDataTreeIterator
{
  public:
                                avtTransform();
    virtual                    ~avtTransform();

    virtual const char         *GetType(void) { return "avtTransform"; };
    virtual const char         *GetDescription(void)
                                        { return "Transforming data"; };

  protected:
    virtual vtkDataSet         *ExecuteData(vtkDataSet *, int, std::string);
    virtual vtkMatrix4x4       *GetTransform() = 0;
    virtual void                UpdateDataObjectInfo(void);

    vtkDataSet                 *TransformRectilinear(vtkRectilinearGrid *);
    bool                        OutputIsRectilinear(vtkMatrix4x4 *);
    vtkDataSet                 *TransformRectilinearToRectilinear(
                                                         vtkRectilinearGrid *);
    vtkDataSet                 *TransformRectilinearToCurvilinear(
                                                         vtkRectilinearGrid *);
    virtual void                TransformData(vtkRectilinearGrid *) {;};
    virtual bool                TransformVectors(void) { return true; };
};


#endif


