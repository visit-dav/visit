/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                             avtDataBinning.C                              //
// ************************************************************************* //

#include <avtDataBinning.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkDataSetWriter.h>

#include <avtBinningScheme.h>
#include <avtDataBinningFunctionInfo.h>
#include <avtParallel.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>

using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtDataBinning constructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtDataBinning::avtDataBinning(avtDataBinningFunctionInfo *info, float *v)
{
    functionInfo = info;
    vals         = v;
}


// ****************************************************************************
//  Method: avtDataBinning destructor
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
// ****************************************************************************

avtDataBinning::~avtDataBinning()
{
    delete [] vals;
    delete functionInfo;
}


// ****************************************************************************
//  Method: avtDataBinning::CreateGrid
//
//  Purpose:
//      Creates a grid so the DataBinning can be visualized.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Aug 21 14:05:14 PDT 2010
//    Changes to setup 1D curves and also to work well inside an operator.
//
// ****************************************************************************

vtkDataSet *
avtDataBinning::CreateGrid(void)
{
    avtBinningScheme *bs = functionInfo->GetBinningScheme();
    vtkDataSet *rv = bs->CreateGrid();
    int nCells = rv->GetNumberOfCells();
    int nPts   = rv->GetNumberOfPoints();
    int nvals = (bs->GetNumberOfDimensions() == 1 ? nPts : nCells);

    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetName(functionInfo->GetCodomainName().c_str());
    arr->SetNumberOfTuples(nvals);
    for (int i = 0 ; i < nvals ; i++)
    {
        float loc[3];
        if (bs->GetNumberOfDimensions() == 1)
        {
            double *pt = rv->GetPoint(i);
            loc[0] = pt[0];
            loc[1] = pt[1];
            loc[2] = pt[2];
        }
        else
        {
            vtkCell *cell = rv->GetCell(i);
            double bbox[6];
            cell->GetBounds(bbox);
    
            // Note the assumption here that the X-axis corresponds to tuple 0,
            // the Y-axis to tuple 1, etc.
            loc[0] = (bbox[0] + bbox[1]) / 2.;
            loc[1] = (bbox[2] + bbox[3]) / 2.;
            loc[2] = (bbox[4] + bbox[5]) / 2.;
        }

        int binId = bs->GetBinId(loc);
        arr->SetValue(i, vals[binId]);
    }

    // We set up curve data as point data, 2D and 3D as cell data.
    if (bs->GetNumberOfDimensions() == 1)
    {
        rv->GetPointData()->AddArray(arr);
        rv->GetPointData()->SetActiveScalars(functionInfo->GetCodomainName().c_str());
    }
    else
    {
        rv->GetCellData()->AddArray(arr);
        rv->GetCellData()->SetActiveScalars(functionInfo->GetCodomainName().c_str());
    }

    arr->Delete();

    return rv;
}


// ****************************************************************************
//  Method: avtDataBinning::ApplyFunction
//
//  Purpose:
//      Applies the DataBinning to a data set.
//
//  Programmer: Hank Childs
//  Creation:   February 12, 2006
//
//  Modifications:
//
//    Hank Childs, Fri Feb 15 16:15:16 PST 2008
//    Fix memory leak with error condition.
//
//    Hank Childs, Mon Jul 16 16:19:51 PDT 2012
//    Add support for coordinates.  Also improve error handling for mixed 
//    centering.
//
// ****************************************************************************

class ValueRetriever
{
  public:
    virtual double GetValue(int) = 0;
    virtual ~ValueRetriever() { ; };
};

class VariableValueRetriever : public ValueRetriever
{
  public:
                   VariableValueRetriever(vtkDataArray *a) { arr = a; };
    double         GetValue(int v) { return arr->GetTuple1(v); };
  private:
    vtkDataArray  *arr;
};

class CoordinateValueRetriever : public ValueRetriever
{
  public:
                   CoordinateValueRetriever(vtkDataSet *m, int c) { mesh = m; coord = c; };
    double         GetValue(int v) { double p[3]; mesh->GetPoint(v, p);  return p[coord]; };

  private:
    vtkDataSet    *mesh;
    int            coord;
};


vtkDataArray *
avtDataBinning::ApplyFunction(vtkDataSet *ds)
{
    int   i, j, k;
    int   nvars = functionInfo->GetDomainNumberOfTuples();

    bool hasError = false;
    int numNodal = 0;

    ValueRetriever **val_ret = new ValueRetriever*[nvars];
    for (k = 0 ; k < nvars ; k++)
         val_ret[k] = NULL;
    for (k = 0 ; k < nvars ; k++)
    {
        avtDataBinningFunctionInfo::BinBasedOn bbo =
                                           functionInfo->GetBinBasedOnType(k);
        if (bbo == avtDataBinningFunctionInfo::VARIABLE)
        {
            const char *varname = functionInfo->GetDomainTupleName(k).c_str();
            vtkDataArray *arr = ds->GetPointData()->GetArray(varname);
            if (arr != NULL)
                numNodal++;
            else
                arr = ds->GetCellData()->GetArray(varname);

            if (arr == NULL) // not in point data or cell data
                hasError = true;
            else
                val_ret[k] = new VariableValueRetriever(arr);
        }
        else 
        {
            numNodal++;
            if (bbo == avtDataBinningFunctionInfo::X)
                val_ret[k] = new CoordinateValueRetriever(ds, 0);
            if (bbo == avtDataBinningFunctionInfo::Y)
                val_ret[k] = new CoordinateValueRetriever(ds, 1);
            if (bbo == avtDataBinningFunctionInfo::Z)
                val_ret[k] = new CoordinateValueRetriever(ds, 2);
        }
    }

    if (0 < numNodal && numNodal < nvars) // mixed centering
    {
        hasError = true;
    }

    if (hasError)
    {
        for (k = 0 ; k < nvars ; k++)
            if (val_ret[k] != NULL)
                delete val_ret[k];
        delete [] val_ret;

        debug1 << "Could not locate one of the tuples from the "
               << "domain.  Or the variables have different centerings."
               << endl;
        // Put burden caller to issue better error message.
        return NULL;
    }

    bool isNodal = (numNodal > 0);
    int nvals = (isNodal ? ds->GetNumberOfPoints() : ds->GetNumberOfCells());
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetName(functionInfo->GetCodomainName().c_str());
    rv->SetNumberOfTuples(nvals);
    float *v = new float[nvars];
    avtBinningScheme *bs = functionInfo->GetBinningScheme();
    for (i = 0 ; i < nvals ; i++)
    {
        for (j = 0 ; j < nvars ; j++)
            v[j] = val_ret[j]->GetValue(i);
        int binId = bs->GetBinId(v);
        rv->SetValue(i, vals[binId]);
    }

    for (k = 0 ; k < nvars ; k++)
        if (val_ret[k] != NULL)
            delete val_ret[k];
    delete [] val_ret;

    delete [] v;

    return rv;
}


// ****************************************************************************
//  Method: avtDataBinning::OutputDataBinning
//
//  Purpose:
//      Outputs a DataBinning.
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
//    Hank Childs, Sat Aug 21 14:05:14 PDT 2010
//    Set up curve with point data.
//
//    Cyrus Harrison, Thu Nov 17 16:13:25 PST 2011
//    Fixed bug with output for 1D grids. For each curve(grid) point we do
//    have a correspdong bin value. We don't need to average grid points.
//
// ****************************************************************************

void
avtDataBinning::OutputDataBinning(const std::string &ddfname)
{
    if (PAR_Rank() != 0)
        return;

    avtDataBinningFunctionInfo *i = GetFunctionInfo();
    int numDims = i->GetDomainNumberOfTuples();
    vtkDataSet *g = CreateGrid();
    if (numDims > 1)
    {
        vtkDataSetWriter *wrtr = vtkDataSetWriter::New();
        char str[1024];
        sprintf(str, "%s.vtk", ddfname.c_str());
        wrtr->SetFileName(str);
        wrtr->SetInputData(g);
        wrtr->Write();
        wrtr->Delete();
    }
    else
    {
        vtkRectilinearGrid *r = (vtkRectilinearGrid *) g;
        int dims[3];
        r->GetDimensions(dims);
        vtkDataArray *s = r->GetPointData()->GetArray(0);
        vtkDataArray *x = r->GetXCoordinates();
        char str[1024];
        sprintf(str, "%s.ultra", ddfname.c_str());
        ofstream ofile(str);
        ofile << "# DataBinning " << ddfname << endl;
        for (int j = 0 ; j < dims[0] ; j++)
        {
            ofile << x->GetTuple1(j) << " "
                  << s->GetTuple1(j) << endl;
        }
    }
    g->Delete();
}


