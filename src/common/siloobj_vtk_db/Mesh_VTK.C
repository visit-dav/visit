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
//                               Mesh_VTK.C                                  //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#include <Mesh_VTK.h>
#include <SiloObjLib.h>
#include <TableOfContents.h>


// ****************************************************************************
//  Method: Mesh_VTK constructor
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2000
//
// ****************************************************************************

Mesh_VTK::Mesh_VTK()
{
    toc         = NULL;
    coordsField = NULL;
}


// ****************************************************************************
//  Method: Mesh_VTK destructor
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2000
//
// ****************************************************************************

Mesh_VTK::~Mesh_VTK()
{
    if (coordsField != NULL)
    {
        delete coordsField;
    } 
}


// ****************************************************************************
//  Method: Mesh_VTK::UpdateReferences
//
//  Purpose:
//      Allows Mesh to populate its data members with the constructed objects
//      in the TOC.  Also, get a reference to the TOC.
//
//  Arguments:
//      t      The TableOfContents object that now points to this Mesh.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2000
//
// ****************************************************************************

void
Mesh_VTK::UpdateReferences(TableOfContents *t)
{
    toc         = t;
    coordsField = toc->GetField(coordinates);
}


// ****************************************************************************
//  Method: Mesh_VTK::GetCoords
//
//  Purpose:
//      Creates a list of coordinates.
//
//  Arguments:
//      list     A list of the domains.
//      listN    The number of domains in list.
//      ugrid    The unstructured grid to be populated.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2000
//
//  Modifications:
// 
//      Hank Childs, Mon Apr  3 16:45:36 PDT 2000
//      Changed input to be an unstructured grid instead of vtkPoints so that
//      this routine would match the one for other grid types.
//
// ****************************************************************************

void
Mesh_VTK::GetCoords(const int *list, int listN, vtkUnstructuredGrid **ugrid)
{
    //
    // Get the list of coordinates from the Field->Value.
    // Accessing GetValueDomains through friend status to Field_VTK.
    //
    if (coordsField == NULL)
    {
        cerr << "Did not find coordinate field, cannot create coords." << endl;
        // throw
        return;
    }

    int     *sizes  = NULL;
    float  **vals   = NULL;
    coordsField->GetValueDomains(list, listN, &vals, &sizes);

    //
    // vals has one entry for each domain.  Inside that there are subarrays
    // for each dimension  vals[i] = { stuff for x, stuff for y, stuff for z }
    // For curvilinear and unstructured meshes, the size of stuff for x will
    // equal the size of stuff for y, etc, so we can make some assumptions
    // when finding the offsets.  sizes has 'coordinateDimension' entries for
    // each domain.  sizes[i][j] = size of stuff for dim j for domain i.
    // Above comment can be condensed as sizes[i][0] = sizes[i][1], etc.
    //
    for (int i = 0 ; i < listN ; i++)
    {
        vtkPoints *points = ugrid[i]->GetPoints();
        for (int j = 0 ; j < sizes[i*coordinateDimension] ; j++)
        {
            float point[3];
            for (int k = 0 ; k < 3 ; k++)
            {
                if (k < coordinateDimension)
                {
                    point[k] = vals[i][j + k*sizes[i*coordinateDimension]];
                }
                else
                {
                    point[k] = 0.;
                }
            }
            points->InsertNextPoint(point);
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
//  Method: Mesh_VTK::GetCoords
//
//  Purpose:
//      Creates a list of coordinates.
//
//  Arguments:
//      list     A list of the domains.
//      listN    The number of domains in list.
//      sgrid    The structured grid to be populated.
//
//  Programmer: Hank Childs
//  Creation:   April 3, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Sep 28 16:08:46 PDT 2000
//    Use the dims object to correctly determine dimensions of grid.
//
// ****************************************************************************

void
Mesh_VTK::GetCoords(const int *list, int listN, vtkStructuredGrid **sgrid)
{
    //
    // Get the list of coordinates from the Field->Value.
    // Accessing GetValueDomains through friend status to Field_VTK.
    //
    if (coordsField == NULL)
    {
        cerr << "Did not find coordinate field, cannot create coords." << endl;
        // throw
        return;
    }
    int     *sizes  = NULL;
    float  **vals   = NULL;
    coordsField->GetValueDomains(list, listN, &vals, &sizes);

    int     *dimsSizes      = NULL;
    float  **typedDimsVals  = NULL;
    coordsField->GetDimsDomains(list, listN, &typedDimsVals, &dimsSizes);
    int    **dimsVals = (int **) typedDimsVals;

    //
    // vals has one entry for each domain.  Inside that there are subarrays
    // for each dimension  vals[i] = { stuff for x, stuff for y, stuff for z }
    // For curvilinear and unstructured meshes, the size of stuff for x will
    // equal the size of stuff for y, etc, so we can make some assumptions
    // when finding the offsets.  sizes has 'coordinateDimension' entries for
    // each domain.  sizes[i][j] = size of stuff for dim j for domain i.
    // Above comment can be condensed as sizes[i][0] = sizes[i][1], etc.
    //
    for (int i = 0 ; i < listN ; i++)
    {
        //
        // Set the dimension of the curvilinear grid.
        //
        int  dims[3];
        for (int k = 0 ; k < 3 ; k++)
        {
            if (k < coordinateDimension)
            {
                dims[k] = dimsVals[i][k];
            }
            else
            {
                dims[k] = 0;
            }
        }
        sgrid[i]->SetDimensions(dims);

        vtkPoints *points = sgrid[i]->GetPoints();
        for (int j = 0 ; j < sizes[i*coordinateDimension] ; j++)
        {
            float point[3];
            for (int k = 0 ; k < 3 ; k++)
            {
                if (k < coordinateDimension)
                {
                    point[k] = vals[i][j + k*sizes[i*coordinateDimension]];
                }
                else
                {
                    point[k] = 0.;
                }
            }
            points->InsertNextPoint(point);
        }
    }

    //
    // Prevent a memory leak.  Note that the pointers in vals are all cached in
    // the Value object, so we cannot delete them, only the array that holds
    // them.
    //
    delete [] sizes;
    delete [] vals;
    delete [] dimsSizes;
    delete [] dimsVals;
}


// ****************************************************************************
//  Method: Mesh_VTK::GetCoords
//
//  Purpose:
//      Gets the coordinates for a rectilinear mesh and adds it straight
//      to the vtkRectilinearGrid.
//
//  Arguments:
//      list     A list of the domains.
//      listN    The number of domains in list.
//      rgrid    The rectilinear grid to be populated.
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2000
//
//  Modifications:
// 
//      Hank Childs, Wed Apr  5 16:12:54 PDT 2000
//      Replaced InsertNextScalar with InsertScalar since dummy values
//      are left prepended in the scalars.
//
// ****************************************************************************

void
Mesh_VTK::GetCoords(const int *list, int listN, vtkRectilinearGrid **rgrid)
{
    //
    // Get the list of coordinates from the Field->Value.
    // Accessing GetValueDomains through friend status to Field_VTK.
    //
    int     *sizes  = NULL;
    float  **vals   = NULL;
    if (coordsField == NULL)
    {
        cerr << "Did not find coordinate field, cannot create coords." << endl;
        // throw
        return;
    }
    coordsField->GetValueDomains(list, listN, &vals, &sizes);

    //
    // vals has one entry for each domain.  Inside that there are subarrays
    // for each dimension  vals[i] = { stuff for x, stuff for y, stuff for z }
    // sizes has 'coordinateDimension' entries for each domain and 
    // sizes[i][j] = size of stuff for dim. j for domain i.
    //
    for (int i = 0 ; i < listN ; i++)
    {
        //
        // Set the dimension of the rectilinear grid.
        //
        int  dims[3];
        for (int k = 0 ; k < 3 ; k++)
        {
            if (k < coordinateDimension)
            {
                dims[k] = sizes[i*coordinateDimension + k];
            }
            else
            {
                dims[k] = 0;
            }
        }
        rgrid[i]->SetDimensions(dims);

        // 
        // Add the points to the rectilinear grid.
        //
        for (int j = 0 ; j < coordinateDimension ; j++)
        {
            vtkScalars *dimArray = vtkScalars::New();
            dimArray->SetNumberOfScalars(sizes[i*coordinateDimension+j]);
            for (int k = 0 ; k < sizes[i*coordinateDimension+j] ; k++)
            {
                dimArray->InsertScalar(k, vals[i]
                                       [sizes[i*coordinateDimension+j] + k]);
            }
            if (j == 0)
            {
                rgrid[i]->SetXCoordinates(dimArray);
            }
            else if (j == 1)
            {
                rgrid[i]->SetYCoordinates(dimArray);
            }
            else
            {
                rgrid[i]->SetZCoordinates(dimArray);
            }
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
//  Method: Mesh_VTK::GetMetaData
//
//  Purpose:
//      Gets the meta data for the coordinates of this Mesh object.
//
//  Returns:    A pointer to a constant IntervalTree_VTK.
//
//  Programmer: Hank Childs
//  Creation:   April 12, 2000
//
// ****************************************************************************

const IntervalTree_VTK *
Mesh_VTK::GetMetaData(void)
{
    if (coordsField == NULL)
    {
        cerr << "Could not establish coordinates to find domains." << endl;
        // throw
        return NULL;
    }

    return coordsField->GetMetaData();
}


