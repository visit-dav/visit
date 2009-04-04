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
//                                avtDDF.C                                   //
// ************************************************************************* //

#include <avtDDF.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkDataSetWriter.h>

#include <avtBinningScheme.h>
#include <avtDDFFunctionInfo.h>
#include <avtParallel.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtDDF constructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtDDF::avtDDF(avtDDFFunctionInfo *info, float *v)
{
    functionInfo = info;
    vals         = v;
}


// ****************************************************************************
//  Method: avtDDF destructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtDDF::~avtDDF()
{
    delete [] vals;
    delete functionInfo;
}


// ****************************************************************************
//  Method: avtDDF::CreateGrid
//
//  Purpose:
//      Creates a grid so the DDF can be visualized.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

vtkDataSet *
avtDDF::CreateGrid(void)
{
    avtBinningScheme *bs = functionInfo->GetBinningScheme();
    vtkDataSet *rv = bs->CreateGrid();
    int nCells = rv->GetNumberOfCells();

    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetName(functionInfo->GetCodomainName().c_str());
    arr->SetNumberOfTuples(nCells);
    for (int i = 0 ; i < nCells ; i++)
    {
        vtkCell *cell = rv->GetCell(i);
        double bbox[6];
        cell->GetBounds(bbox);

        // Note the assumption here that the X-axis corresponds to tuple 0,
        // the Y-axis to tuple 1, etc.
        float center[3];
        center[0] = (bbox[0] + bbox[1]) / 2.;
        center[1] = (bbox[2] + bbox[3]) / 2.;
        center[2] = (bbox[4] + bbox[5]) / 2.;

        int binId = bs->GetBinId(center);
        arr->SetValue(i, vals[binId]);
    }

    rv->GetCellData()->AddArray(arr);
    arr->Delete();

    return rv;
}


// ****************************************************************************
//  Method: avtDDF::ApplyFunction
//
//  Purpose:
//      Applies the DDF to a data set.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Fri Feb 15 16:15:16 PST 2008
//    Fix memory leak with error condition.
//
// ****************************************************************************

vtkDataArray *
avtDDF::ApplyFunction(vtkDataSet *ds)
{
    int   i, j, k;
    int   nvars = functionInfo->GetDomainNumberOfTuples();

    bool hasError = false;
    bool isNodal = true;
    const char   *varname0 = functionInfo->GetCodomainName().c_str();
    vtkDataArray *arr0 = ds->GetPointData()->GetArray(varname0);
    if (arr0 == NULL)
    {
        arr0 = ds->GetCellData()->GetArray(varname0);
        if (arr0 == NULL)
            hasError = true;
        isNodal = false;
    }

    vtkDataArray **arr = new vtkDataArray*[nvars];
    for (k = 0 ; k < nvars ; k++)
    {
        const char *varname = functionInfo->GetDomainTupleName(k).c_str();
        arr[k] = (isNodal ? ds->GetPointData()->GetArray(varname)
                          : ds->GetCellData()->GetArray(varname));
        if (arr[k] == NULL)
        {
            hasError = true;
        }
    }

    if (hasError)
    {
        delete [] arr;
        debug1 << "Could not locate one of the tuples from the "
               << "domain.  Or the variables have different centerings."
               << endl;
        // Put burden caller to issue better error message.
        return NULL;
    }

    int nvals = (isNodal ? ds->GetNumberOfPoints() : ds->GetNumberOfCells());
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetName(functionInfo->GetCodomainName().c_str());
    rv->SetNumberOfTuples(nvals);
    float *v = new float[nvars];
    avtBinningScheme *bs = functionInfo->GetBinningScheme();
    for (i = 0 ; i < nvals ; i++)
    {
        for (j = 0 ; j < nvars ; j++)
            v[j] = arr[j]->GetTuple1(i);
        int binId = bs->GetBinId(v);
        rv->SetValue(i, vals[binId]);
    }

    delete [] arr;
    delete [] v;

    return rv;
}


// ****************************************************************************
//  Method: avtDDF::OutputDDF
//
//  Purpose:
//      Outputs a DDF.
//
//  Programmer: Hank Childs
//  Creation:   March 30, 2006
//
//  Modifications:
//
//    Hank Childs, Thu Mar 26 13:15:10 CDT 2009
//    Only have the root processor write out the file (wowsa! -- how did this
//    not get noticed for 3 years!).
//
// ****************************************************************************

void
avtDDF::OutputDDF(const std::string &ddfname)
{
    if (PAR_Rank() != 0)
        return;

    avtDDFFunctionInfo *i = GetFunctionInfo();
    int numDims = i->GetDomainNumberOfTuples();
    vtkDataSet *g = CreateGrid();
    if (numDims > 1)
    {
        vtkDataSetWriter *wrtr = vtkDataSetWriter::New();
        char str[1024];
        sprintf(str, "%s.vtk", ddfname.c_str());
        wrtr->SetFileName(str);
        wrtr->SetInput(g);
        wrtr->Write();
        wrtr->Delete();
    }
    else
    {
        vtkRectilinearGrid *r = (vtkRectilinearGrid *) g;
        int dims[3];
        r->GetDimensions(dims);
        vtkDataArray *s = r->GetCellData()->GetArray(0);
        vtkDataArray *x = r->GetXCoordinates();
        char str[1024];
        sprintf(str, "%s.ultra", ddfname.c_str());
        ofstream ofile(str);
        ofile << "# DDF " << ddfname << endl;
        for (int j = 0 ; j < dims[0]-1 ; j++)
        {
            ofile << (x->GetTuple1(j) + x->GetTuple1(j+1)) / 2. << " "
                  << s->GetTuple1(j) << endl;
        }
    }
    g->Delete();
}


