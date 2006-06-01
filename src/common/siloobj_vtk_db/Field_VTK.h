/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

// ************************************************************************* //
//                              Field_VTK.h                                  //
// ************************************************************************* //

#ifndef FIELD_VTK_H
#define FIELD_VTK_H
#include <siloobj_vtk_exports.h>

#include <vtkScalars.h>

#include <Field.h>
#include <IntervalTree_VTK.h>
#include <Value_VTK.h>


//
// Forward declaration of classes.
//

class  TableOfContents;


// ****************************************************************************
//  Class: Field_VTK
// 
//  Purpose:
//      A derived class of Field that has an understanding of VTK.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2000
//
//  Modifications:
//  
//     Hank Childs, Wed Apr 12 21:45:33 PDT 2000
//     Removed methods GetDomainList and GetRange, added method GetMetaData.
//
// ****************************************************************************

class SILOOBJ_VTK_API Field_VTK : public Field
{
    friend                    class Mesh_VTK;

  public:
                              Field_VTK();
    virtual                  ~Field_VTK();

    const IntervalTree_VTK   *GetMetaData(void);
    const char               *GetMeshName(void);
    void                      GetVar(const int *, int, vtkScalars **);

    void                      UpdateReferences(TableOfContents *);

  protected:
    // A reference to the TOC that holds it.
    TableOfContents          *toc;

    // A properly typed alias to the base class' mixed value object that allows
    // it to act as a _VTK object. 
    Value_VTK                *aliasedMixedValueObject;

    // A properly typed alias to the base class' value object that allows it 
    // to act as a _VTK object. 
    Value_VTK                *aliasedValueObject;

    // A properly typed alias to the base class' value object that allows it 
    // to act as a _VTK object. 
    Value_VTK                *aliasedDimsObject;

    // A properly typed alias to the base class' interval tree object that 
    // allows it to act as a _VTK object. 
    IntervalTree_VTK         *aliasedIntervalTreeObject;

    void                      CreateValues(Value_VTK *, Value_VTK *,Value_VTK *,
                                           IntervalTree_VTK *);
    void                      GetValueDomains(const int *, int, float ***, 
                                              int **);
    void                      GetDimsDomains(const int *, int, float ***, 
                                              int **);
};


#endif


