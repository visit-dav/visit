// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtCurve2DWriter.C                           //
// ************************************************************************* //

#include <avtCurve2DWriter.h>

#include <vector>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>

#include <avtDatabaseMetaData.h>
#include <StringHelpers.h>

#include <DBOptionsAttributes.h>

using     std::string;
using     std::vector;

// ****************************************************************************
//  Method: avtCurve2DWriter constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 26 16:32:43 PST 2012
//
//  Modifications:
//    Kathleen Biagas, Fri Aug 31 14:18:46 PDT 2018
//    Read comment style from write options.
//
// ****************************************************************************

avtCurve2DWriter::avtCurve2DWriter(DBOptionsAttributes *atts) : commentStyle("#")
{
    nBlocks = 0;
    if (atts->GetEnum("CommentStyle") == 1)
        commentStyle = "%";
}

// ****************************************************************************
//  Method: avtCurve2DWriter::OpenFile
//
//  Purpose:
//      Does no actual work.  Just records the stem name for the files.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 26 16:32:43 PST 2012
//
// ****************************************************************************

void
avtCurve2DWriter::OpenFile(const string &stemname, int numblocks)
{
    stem = stemname;
    nBlocks = numblocks;
}


// ****************************************************************************
//  Method: avtCurve2DWriter::WriteHeaders
//
//  Purpose:
//      Writes out a VisIt file to tie the Curve2D files together.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 26 16:32:43 PST 2012
//
// ****************************************************************************

void
avtCurve2DWriter::WriteHeaders(const avtDatabaseMetaData *md,
                           const vector<string> &scalars,
                           const vector<string> &vectors,
                           const vector<string> &materials)
{
}

// ****************************************************************************
// Method: avtCurve2DWriter::SanitizeName
//
// Purpose: 
//   Sanitize the variable name.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 27 11:14:10 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

std::string
avtCurve2DWriter::SanitizeName(const std::string &name) const
{
    std::string retval(name);

    if(name.find("operators/") != std::string::npos)
    {
        std::vector<std::string> tokens = StringHelpers::split(retval, '/');
        if(!tokens.empty())
            retval = tokens[tokens.size()-1];
    }

    return retval;
}

// ****************************************************************************
//  Method: avtCurve2DWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 26 16:32:43 PST 2012
//
// ****************************************************************************

void
avtCurve2DWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    char name[1024];
    if (nBlocks == 1)
        snprintf(name, 1024, "%s.curve", stem.c_str());
    else
        snprintf(name, 1024, "%s.%03d.curve", stem.c_str(), chunk);

    ofstream ofile(name);
    if (ofile.fail())
        EXCEPTION0(ImproperUseException);

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if(atts.CycleIsAccurate())
        ofile << commentStyle << " CYCLE " << atts.GetCycle() << endl;
    if(atts.TimeIsAccurate())
        ofile << commentStyle << " TIME " << atts.GetTime() << endl;

    bool isCurve = false;
    vtkRectilinearGrid *rgrid = vtkRectilinearGrid::SafeDownCast(ds);
    int dims[3] = {0,0,0};
    if(rgrid != NULL)
    {
        rgrid->GetDimensions(dims);
        isCurve = (dims[1] <= 1 && dims[2] <= 1 && rgrid->GetXCoordinates() != NULL);
    }
    if(isCurve)
    {
        vtkDataArray *xc = rgrid->GetXCoordinates();
        for(int i = 0; i < rgrid->GetPointData()->GetNumberOfArrays(); ++i)
        {
            vtkDataArray *arr = rgrid->GetPointData()->GetArray(i);
            ofile << commentStyle << " " << SanitizeName(arr->GetName()) << endl;
            for(vtkIdType j = 0; j < arr->GetNumberOfTuples(); ++j)
                ofile << xc->GetTuple1(j) << " " << arr->GetTuple1(j) << endl;
        }
        for(int i = 0; i < rgrid->GetCellData()->GetNumberOfArrays(); ++i)
        {
            vtkDataArray *arr = rgrid->GetCellData()->GetArray(i);
            ofile << commentStyle << " " << SanitizeName(arr->GetName()) << endl;
            for(vtkIdType j = 0; j < arr->GetNumberOfTuples(); ++j)
                ofile << j << " " << arr->GetTuple1(j) << endl;
        }
    }
    else
    {
        for(int i = 0; i < ds->GetPointData()->GetNumberOfArrays(); ++i)
        {
            vtkDataArray *arr = ds->GetPointData()->GetArray(i);
            ofile << commentStyle << " " << SanitizeName(arr->GetName()) << endl;
            for(vtkIdType j = 0; j < arr->GetNumberOfTuples(); ++j)
                ofile << j << " " << arr->GetTuple1(j) << endl;
        }
        for(int i = 0; i < ds->GetCellData()->GetNumberOfArrays(); ++i)
        {
            vtkDataArray *arr = ds->GetCellData()->GetArray(i);
            ofile << commentStyle << " " << SanitizeName(arr->GetName()) << endl;
            for(vtkIdType j = 0; j < arr->GetNumberOfTuples(); ++j)
                ofile << j << " " << arr->GetTuple1(j) << endl;
        }
    }
}


// ****************************************************************************
//  Method: avtCurve2DWriter::CloseFile
//
//  Purpose:
//      Closes the file.  This does nothing in this case.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 26 16:32:43 PST 2012
//
// ****************************************************************************

void
avtCurve2DWriter::CloseFile(void)
{
    // CLOSE FILES
}
