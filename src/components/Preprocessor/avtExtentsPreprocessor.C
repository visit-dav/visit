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

// ************************************************************************* //
//                           avtExtentsPreprocessor.C                        //
// ************************************************************************* //

#include <avtExtentsPreprocessor.h>

#include <float.h>

#include <vtkDataSet.h>

#include <avtCommonDataFunctions.h>


// ****************************************************************************
//  Method: avtExtentsPreprocessor constructor
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

avtExtentsPreprocessor::avtExtentsPreprocessor()
{
    varExtents     = NULL;
    spatialExtents = NULL;
}


// ****************************************************************************
//  Method: avtExtentsPreprocessor destructor
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

avtExtentsPreprocessor::~avtExtentsPreprocessor()
{
    if (varExtents != NULL)
    {
        delete [] varExtents;
        varExtents = NULL;
    }
    if (spatialExtents != NULL)
    {
        delete [] spatialExtents;
        spatialExtents = NULL;
    }
}


// ****************************************************************************
//  Method: avtExtentsPreprocessor::Initialize
//
//  Purpose:
//      Called before the domains are processed.  This allows for the extents
//      to be set up correctly, etc.
//
//  Arguments:
//      nd      The number of domains to be processed 
//                  (with nd-1 being the biggest).
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// **************************************************************************** 

void
avtExtentsPreprocessor::Initialize(int nd)
{
    numDomains = nd;

    varExtents = new double[2*numDomains];
    spatialExtents = new double[6*numDomains];

    for (int i = 0 ; i < numDomains ; i++)
    {
        varExtents[2*i]   = +DBL_MAX;
        varExtents[2*i+1] = -DBL_MAX;
        for (int j = 0 ; j < 3 ; j++)
        {
            spatialExtents[2*i+2*j]   = +DBL_MAX;
            spatialExtents[2*i+2*j+1] = -DBL_MAX;
        }
    }
}


// ****************************************************************************
//  Method: avtExtentsPreprocessor::ProcessDomain
//
//  Purpose:
//      Gets the extents of each domain and adds it to our list.
//
//  Arguments:
//      ds      The VTK dataset to get the extents from.
//      dom     The index of that domain.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

void
avtExtentsPreprocessor::ProcessDomain(vtkDataSet *ds, int dom)
{
    float bounds[6];
    ds->GetBounds(bounds);

    for (int i = 0 ; i < 3 ; i++)
    {
        if (spatialExtents[6*dom + 2*i] > bounds[2*i])
        {
            spatialExtents[6*dom + 2*i] = bounds[2*i];
        }
        if (spatialExtents[6*dom + 2*i + 1] < bounds[2*i + 1])
        {
            spatialExtents[6*dom + 2*i + 1] = bounds[2*i + 1];
        }
    }

    float range[2];
    ds->GetScalarRange(range);

    if (varExtents[2*dom] > range[0])
    {
        varExtents[2*dom] = range[0];
    }
    if (varExtents[2*dom + 1] < range[1])
    {
        varExtents[2*dom + 1] = range[1];
    }
}


// ****************************************************************************
//  Method: avtExtentsPreprocessor::Finalize
//
//  Purpose:
//      Gives the preprocessor module a chance to determine the final extents.
//
//  Programmer: Hank Childs
//  Creation:   September 9, 2001
//
// ****************************************************************************

void
avtExtentsPreprocessor::Finalize(void)
{
    double finalBounds[6] = { +DBL_MAX, -DBL_MAX, +DBL_MAX, -DBL_MAX,
                              +DBL_MAX, -DBL_MAX };
    double finalRange[2]  = { +DBL_MAX, -DBL_MAX };
    for (int i = 0 ; i < numDomains ; i++)
    {
        cerr << "Bounds for domain " << i << ": ";
        for (int j = 0 ; j < 3 ; j++)
        {
            cerr << "( " << spatialExtents[6*i + 2*j] << ", " 
                 << spatialExtents[6*i + 2*j+1] << "), ";
            if (spatialExtents[6*i + 2*j] < finalBounds[2*j])
            {
                finalBounds[2*j] = spatialExtents[6*i + 2*j];
            }
            if (spatialExtents[6*i + 2*j + 1] > finalBounds[2*j + 1])
            {
                finalBounds[2*j + 1] = spatialExtents[6*i + 2*j + 1];
            }
        }
        cerr << endl;

        cerr << "Range for domain " << i << ": " << varExtents[2*i] << ", "
             << varExtents[2*i+1] << endl;
        if (varExtents[2*i] < finalRange[0])
        {
            finalRange[0] = varExtents[2*i];
        }
        if (varExtents[2*i + 1] > finalRange[1])
        {
            finalRange[1] = varExtents[2*i + 1];
        }
    }
 
    cerr << "Final range = " << finalRange[0] << " - " << finalRange[1] 
         << endl;
    cerr << "Final bounds = " << finalBounds[0] << " - " << finalBounds[1]
         << ", " << finalBounds[2] << " - " << finalBounds[3] << ", "
         << finalBounds[4] << " - " << finalBounds[5] << endl;
}


