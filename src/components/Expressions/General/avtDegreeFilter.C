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
//                             avtDegreeFilter.C                             //
// ************************************************************************* //

#include <avtDegreeFilter.h>

#include <vtkCell.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDegreeFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDegreeFilter::avtDegreeFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDegreeFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDegreeFilter::~avtDegreeFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDegreeFilter::DeriveVariable
//
//  Purpose:
//      Derives a variable based on the input dataset.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     June 7, 2002
//
// ****************************************************************************

vtkDataArray *
avtDegreeFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int  i, j;

    //
    // Set up an array that we can do bookkeepping in.  Entry i will be the
    // number of cells incident to point i.
    //
    int nPoints = in_ds->GetNumberOfPoints();
    int *degree = new int[nPoints];
    for (i = 0 ; i < nPoints ; i++)
    {
        degree[i] = 0;
    }

    //
    // Iterate over each cell in the mesh and ask it which points it is
    // incident to.  Reflect that in our counts of how many cells each point
    // is incident to (ie the degree).
    //
    int nCells = in_ds->GetNumberOfCells();
    for (i = 0 ; i < nCells ; i++)
    {
        vtkCell *cell = in_ds->GetCell(i);
        int numPointsForThisCell = cell->GetNumberOfPoints();
        for (j = 0 ; j < numPointsForThisCell ; j++)
        {
            int id = cell->GetPointId(j);
            if (id >= 0 && id < nPoints)
            {
                degree[id]++;
            }
        }
    }

    //
    // Set up a VTK variable reflecting the degrees we have calculated.
    //
    vtkFloatArray *dv = vtkFloatArray::New();
    dv->SetNumberOfTuples(nPoints);
    for (i = 0 ; i < nPoints ; i++)
    {
        float f = (float) degree[i];
        dv->SetTuple(i, &f);
    }
    delete [] degree;

    return dv;
}


