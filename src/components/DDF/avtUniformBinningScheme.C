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
//                          avtUniformBinningScheme.C                        //
// ************************************************************************* //

#include <avtUniformBinningScheme.h>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtUniformBinningScheme constructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtUniformBinningScheme::avtUniformBinningScheme(int nt, const float *r,
                                                 const int *nv)
{
    int  i;

    if (nt <= 0)
    {
        // Must be a positive number of tuples
        EXCEPTION0(ImproperUseException);
    }
    for (i = 0 ; i < nt ; i++)
    {
        if (r[2*i+1] <= r[2*i])
        {
            // Ranges must be ordered correctly.
            EXCEPTION0(ImproperUseException);
        }
        if (nv[i] <= 0)
        {
            // Must have at least one bin.
            EXCEPTION0(ImproperUseException);
        }
    }
    
    ntuples = nt;
    ranges  = new float[2*nt];
    nvals   = new int[nt];
    nBins   = 1;
    for (i = 0 ; i < ntuples ; i++)
    {
        ranges[2*i]   = r[2*i];
        ranges[2*i+1] = r[2*i+1];
        nvals[i]      = nv[i];
        nBins        *= nvals[i];
    }
}

// ****************************************************************************
//  Method: avtUniformBinningScheme destructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtUniformBinningScheme::~avtUniformBinningScheme()
{
    delete [] ranges;
    delete [] nvals;
}


// ****************************************************************************
//  Method: avtUniformBinningScheme::GetBinId
//
//  Purpose:
//      Gets the bin identifier for some set of tuples.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Thu Mar 30 12:38:14 PST 2006
//    Fix precision issue.
//
// ****************************************************************************

int
avtUniformBinningScheme::GetBinId(const float *f) const
{
    int mult = 1;
    int rv = 0;
    for (int i = 0 ; i < ntuples ; i++)
    {
        double span = ranges[2*i+1] - ranges[2*i];
        double dist = f[i] - ranges[2*i];
        int tup_bin = (int)((dist / span)*nvals[i]);
        tup_bin = (tup_bin < 0 ? 0 : tup_bin);
        tup_bin = (tup_bin >= nvals[i] ? nvals[i]-1 : tup_bin);
         
        rv += tup_bin*mult;
        mult *= nvals[i];
    }

    return rv;
}


// ****************************************************************************
//  Method: avtUniformBinningScheme::CreateGrid
//
//  Purpose:
//      Creates a grid corresponding to the binning scheme that allows for
//      direct visualization.
//
//  Notes:      The X-axis must correspond to tuple 0, the Y- to tuple 1, etc.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

vtkDataSet *
avtUniformBinningScheme::CreateGrid(void) const
{
    int  i, j;

    if (ntuples > 3)
    {
        EXCEPTION0(ImproperUseException);
    }

    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
    int dims[3] = { 1, 1, 1 };

    for (i = 0 ; i < 3 ; i++)
    {
        vtkFloatArray *arr = vtkFloatArray::New();
        if (i < ntuples)
        {
            arr->SetNumberOfTuples(nvals[i]+1);
            float start = ranges[2*i];
            float stop  = ranges[2*i+1];
            float step  = (stop - start) / (nvals[i]);
            for (j = 0 ; j < nvals[i]+1 ; j++)
            {
                arr->SetValue(j, start + j*step);
            }
        }
        else
        {
            arr->SetNumberOfTuples(1);
            arr->SetValue(0, 0.);
        }
        dims[i] = arr->GetNumberOfTuples();
        if (i == 0)
            rgrid->SetXCoordinates(arr);
        else if (i == 1)
            rgrid->SetYCoordinates(arr);
        else if (i == 2)
            rgrid->SetZCoordinates(arr);
        arr->Delete();
    }

    rgrid->SetDimensions(dims);
    return rgrid;
}


