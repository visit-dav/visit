/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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
//                       UnstructuredTopology_VTK.C                          //
// ************************************************************************* //

#include <stdlib.h>
#include <string.h>

#include <UnstructuredTopology_VTK.h>
#include <TableOfContents.h>

#include <VTKUtility.h>


// ****************************************************************************
//  Method: UnstructuredTopology_VTK constructor
//
//  Purpose:
//      Creates a normally constructed UnstructuredTopology_VTK object.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2000
//
// ****************************************************************************

UnstructuredTopology_VTK::UnstructuredTopology_VTK() 
     : UnstructuredTopology(DERIVED_TYPE_CONSTRUCTOR)
{
    toc = NULL;

    //
    // CreateValues will be called by the UpdateReferences method.
    //
}


// ****************************************************************************
//  Method: UnstructuredTopology_VTK::CreateValues
//
//  Purpose:
//      Creates the Value objects for the UnstructuredTopology_VTK.
//
//  Arguments:
//      v1      The correctly type node list value.
//      v2      The correctly type shape count value.
//      v3      The correctly type shape type value.
//      v4      The correctly type shape size value.
//
//  Note:       The necessity of this method is discussed in the constructor.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

void
UnstructuredTopology_VTK::CreateValues(Value_VTK *v1, Value_VTK *v2, 
                                       Value_VTK *v3, Value_VTK *v4)
{
    aliasedNodeListValue    = v1;
    aliasedShapeCountValue  = v2;
    aliasedShapeTypeValue   = v3;
    aliasedShapeSizeValue   = v4;

    //
    // We have created the value objects we would like to be our Values.  We
    // cannot use valueObject and mixedValueObject because they are the wrong
    // type.  Alias them here so that we can use the methods for a Value_VTK,
    // but the base methods for UnstructuredTopology will still have access to
    // the same object.
    //
    nodeListValue      = aliasedNodeListValue;
    shapeCountValue    = aliasedShapeCountValue;
    shapeTypeValue     = aliasedShapeTypeValue;
    shapeSizeValue     = aliasedShapeSizeValue;
}


// ****************************************************************************
//  Method: UnstructuredTopology_VTK destructor
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2000
//
// ****************************************************************************

UnstructuredTopology_VTK::~UnstructuredTopology_VTK()
{
    //
    // All of the aliased values will be deleted by the base destructor.
    //
}


// ***************************************************************************
//  Method: UnstructuredTopology_VTK::UpdateReferences
//
//  Purpose:
//      Allows the pointers to Value objects to be updated and the 
//      unstructured topology object to grab a reference to the TOC.
//
//  Programmer: Hank Childs
//  Creation:   February 1, 2000
//
// ****************************************************************************

void
UnstructuredTopology_VTK::UpdateReferences(TableOfContents *t)
{
    toc = t;

    Value_VTK *v1   = toc->GetValue(nodeList);
    Value_VTK *v2   = toc->GetValue(shapeCount);
    Value_VTK *v3   = toc->GetValue(shapeType);
    Value_VTK *v4   = toc->GetValue(shapeSize);

    CreateValues(v1, v2, v3, v4);
}


// ****************************************************************************
//  Method: UnstructuredTopology_VTK::GetZones
//
//  Purpose:
//      Gets the arrays from the domains specified from a node list object.
//
//  Arguments:
//      list    The list of domains of interest.
//      listN   The size of list.
//      ugrid   The unstructured grids to put the connectivity information 
//              into.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Sep 26 08:41:57 PDT 2000
//    Allowed for non-hexahedral zones.
//
// ****************************************************************************

void
UnstructuredTopology_VTK::GetZones(const int *list, int listN, 
                                   vtkUnstructuredGrid **ugrid)
{
    float **typedNLVals  = NULL;
    int    *NLsizes      = NULL;
    aliasedNodeListValue->GetDomains(list, listN, &typedNLVals, &NLsizes);
    int   **NLvals = (int **)(typedNLVals);

    float **typedSCVals  = NULL;
    int    *SCsizes      = NULL;
    aliasedShapeCountValue->GetDomains(list, listN, &typedSCVals, &SCsizes);
    int   **SCvals = (int **)(typedSCVals);

    float **typedSTVals  = NULL;
    int    *STsizes      = NULL;
    aliasedShapeTypeValue->GetDomains(list, listN, &typedSTVals, &STsizes);
    int   **STvals = (int **)(typedSTVals);

    float **typedSSVals  = NULL;
    int    *SSsizes      = NULL;
    aliasedShapeSizeValue->GetDomains(list, listN, &typedSSVals, &SSsizes);
    int   **SSvals = (int **)(typedSSVals);

    for (int i = 0 ; i < listN ; i++)
    {
        int  *nodeList = NLvals[i];
        for (int j = 0 ; j < SCsizes[i] ; j++)
        {
            int   VTK_type = SiloZoneTypeToVTKZoneType(STvals[i][j]);
            for (int k = 0 ; k < SCvals[i][j] ; k++)
            {
                if (VTK_type != VTK_WEDGE)
                {
                    ugrid[i]->InsertNextCell(VTK_type, SSvals[i][j], nodeList);
                }
                else
                {
                    int  VTKWedge[6];
                    TranslateSiloWedgeToVTKWedge(nodeList, VTKWedge);
                    ugrid[i]->InsertNextCell(VTK_type, SSvals[i][j], VTKWedge);
                }
                nodeList += SSvals[i][j];
            }
        }
    }

    //
    // Prevent a memory leak.  Note that the pointers in vals are all cached in
    // the Value object, so we cannot delete them, only the array that holds 
    // them.
    //
    delete [] NLsizes;
    delete [] NLvals;
    delete [] SCsizes;
    delete [] SCvals;
    delete [] STsizes;
    delete [] STvals;
    delete [] SSvals;
    delete [] SSsizes;
}


