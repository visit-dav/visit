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
//                              Field_VTK.C                                  //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#include <Field_VTK.h>
#include <SiloObjLib.h>
#include <TableOfContents.h>


// ****************************************************************************
//  Method: Field_VTK constructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

Field_VTK::Field_VTK() : Field(DERIVED_TYPE_CONSTRUCTOR)
{
    //
    // All of the aliased object are set up when the references are updated
    // by the toc.
    //
}


// ****************************************************************************
//  Method: Field_VTK::CreateValues
//
//  Purpose:
//      Creates the Value objects for the Field.  
//
//  Arguments:
//      v1      The correctly typed value object that will be aliased.
//      v2      The correctly typed dims object that will be aliased.
//      v3      The correctly typed mixed value object that will be aliased.
//      it      The correctly typed interval tree object that will be aliased.
//
//  Note:       The necessity of this method is discussed in the constructor
//              of the base class.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Sep 28 15:06:14 PDT 2000
//    Add value for dims.
//
// ****************************************************************************

void
Field_VTK::CreateValues(Value_VTK *v1, Value_VTK *v2, Value_VTK *v3, 
                        IntervalTree_VTK *it)
{
    aliasedValueObject        = v1;
    aliasedDimsObject         = v2;
    aliasedMixedValueObject   = v3;
    aliasedIntervalTreeObject = it;

    //
    // We have created the value objects we would like to be our Values.  We
    // cannot use valueObject and mixedValueObject because they are the wrong
    // type.  Alias them here so that we can use the methods for a Value_VTK,
    // but the base methods for Field will still have access to the same
    // object.
    //
    valueObject         = aliasedValueObject;
    dimsObject          = aliasedDimsObject;
    mixedValueObject    = aliasedMixedValueObject;
    intervalTreeObject  = aliasedIntervalTreeObject;
}


// ****************************************************************************
//  Method: Field_VTK destructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2000
//
// ****************************************************************************

Field_VTK::~Field_VTK()
{
    //
    // Do not delete any of the aliased objects since they will be destructed
    // by the base class's destructor.
    //

    // 
    // Do not delete the toc since it is not contained by this class.
    //
}


// ****************************************************************************
//  Method: Field_VTK::UpdateReferences
//
//  Purpose:
//      Allows Field to populate its data members with the constructed objects
//      in the TOC.  Also, get a reference to the TOC.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Sep 28 15:07:42 PDT 2000
//    Get dims object from toc.
//
// ****************************************************************************

void
Field_VTK::UpdateReferences(TableOfContents *t)
{
    toc = t;

    Value_VTK *v1         = toc->GetValue(values);
    Value_VTK *v2         = toc->GetValue(dims);
    Value_VTK *v3         = toc->GetValue(mixedValues);
    IntervalTree_VTK *it  = toc->GetIntervalTree(intervalTree);

    CreateValues(v1, v2, v3, it);
}


// ****************************************************************************
//  Method: Field_VTK::GetValueDomains
//
//  Purpose:
//      Gets the arrays for the Value object.
//
//  Arguments:
//      domains    The list of domains of interest.
//      listN      The number of domains in the previous argument.
//      vals       The address to put the values in.
//      sizes      The address to put the sizes of vals in.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

void
Field_VTK::GetValueDomains(const int *domains, int listN, float ***vals, 
                           int **sizes)
{
    aliasedValueObject->GetDomains(domains, listN, vals, sizes);
}


// ****************************************************************************
//  Method: Field_VTK::GetDimsDomains
//
//  Purpose:
//      Gets the arrays for the Dims object.
//
//  Arguments:
//      domains    The list of domains of interest.
//      listN      The number of domains in the previous argument.
//      vals       The address to put the values in.
//      sizes      The address to put the sizes of vals in.
//
//  Programmer: Hank Childs
//  Creation:   September 28, 2000
//
// ****************************************************************************

void
Field_VTK::GetDimsDomains(const int *domains, int listN, float ***vals, 
                           int **sizes)
{
    aliasedDimsObject->GetDomains(domains, listN, vals, sizes);
}


// ****************************************************************************
//  Method: Field_VTK::GetMetaData
//
//  Purpose:
//      Gets the meta data (interval tree) associated with the Field_VTK.
//
//  Returns:    The pointer to a constant interval tree.
//
//  Programmer: Hank Childs
//  Creation:   April 12, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Sep  1 09:11:41 PDT 2000
//    Removed error statement for now acceptable case.
//
// ****************************************************************************

const IntervalTree_VTK *
Field_VTK::GetMetaData(void)
{
    if (aliasedIntervalTreeObject == NULL)
    {
        return NULL;
    }

    return aliasedIntervalTreeObject;
}


// ****************************************************************************
//  Method: Field_VTK::GetVar
//
//  Purpose:
//      Populates a vtkScalars object with information from a Value_VTK.
//
//  Arguments:
//      list     A list of domains of interest.
//      listN    The size of list.
//      scalars  The vtk object where the field's values for the specified
//               domains will be entered.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2000
//
//  Modifications:
//      Hank Childs, Wed Apr  5 16:12:54 PDT 2000
//      Replaced InsertNextScalar with InsertScalar since the InsertNextScalar
//      leaves in dummy values put in at creation.
//    
// ****************************************************************************

void
Field_VTK::GetVar(const int *list, int listN, vtkScalars **scalars)
{
    //
    // Get the list of scalars from the Value.
    //  
    int     *sizes  = NULL;
    float  **vals   = NULL;
    if (aliasedValueObject == NULL)
    {
        cerr << "Field cannot find value object to create scalars." << endl;
        // throw
        return;
    }
    aliasedValueObject->GetDomains(list, listN, &vals, &sizes);

    //
    // HANK - assumes scalar variables - this is probably ok for now.
    //
    for (int i = 0 ; i < listN ; i++)
    {
        for (int j = 0 ; j < sizes[i] ; j++)
        {
            scalars[i]->InsertScalar(j, vals[i][j]);
        }
    }

    //
    // Prevent a memory leak.  Note that the pointers in vals are all cached in
    // the Value object, so we cannot delete them, only the array that holds
    // them.
    //
    delete [] sizes;
    delete [] vals;

}


// ****************************************************************************
//  Method: Field_VTK::GetMeshName
//
//  Purpose:
//      Returns the mesh name.
//
//  Returns:    The name of the mesh associated with this variable.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2000
//
// ****************************************************************************

const char *
Field_VTK::GetMeshName(void)
{
    return mesh;
}


