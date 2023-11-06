// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtCurve3DWriter.C                           //
// ************************************************************************* //

#include <avtCurve3DWriter.h>

#include <vector>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <StringHelpers.h>

#include <DBOptionsAttributes.h>

using     std::string;
using     std::vector;

// ****************************************************************************
//  Method: avtCurve3DWriter constructor
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Aug 31, 2018 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
// ****************************************************************************

avtCurve3DWriter::avtCurve3DWriter(const DBOptionsAttributes *atts) : commentStyle("#")
{
    nBlocks = 0;
    if (atts->GetEnum("CommentStyle") == 1)
        commentStyle = "%";
}

// ****************************************************************************
//  Method: avtCurve3DWriter::OpenFile
//
//  Purpose:
//      Does no actual work.  Just records the stem name for the files.
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Jul 26, 2012 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
// ****************************************************************************

void
avtCurve3DWriter::OpenFile(const string &stemname, int numblocks)
{
    stem = stemname;
    nBlocks = numblocks;
}


// ****************************************************************************
//  Method: avtCurve3DWriter::WriteHeaders
//
//  Purpose:
//      Writes out a VisIt file to tie the Curve3D files together.
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Jul 26, 2012 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
// ****************************************************************************

void
avtCurve3DWriter::WriteHeaders(const avtDatabaseMetaData *md,
                           const vector<string> &scalars,
                           const vector<string> &vectors,
                           const vector<string> &materials)
{
}

// ****************************************************************************
// Method: avtCurve3DWriter::SanitizeName
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
// Programmer: Olivier Cessenat
// Creation:   Oct 5, 2023
//
// Copied from Curve2D plugin as of Jul 27, 2012 and adjusted for 3D
// and make a global mesh with materials out of the figures.
//   
// ****************************************************************************

std::string
avtCurve3DWriter::SanitizeName(const std::string &name) const
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
//  Method: avtCurve3DWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Jul 26, 2012 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
// ****************************************************************************

void
avtCurve3DWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    char name[1024];
    if (nBlocks == 1)
        snprintf(name, 1024, "%s.curve3d", stem.c_str());
    else
        snprintf(name, 1024, "%s.%03d.curve3d", stem.c_str(), chunk);

    ofstream ofile(name);
    if (ofile.fail())
        EXCEPTION0(ImproperUseException);

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if(atts.CycleIsAccurate())
        ofile << commentStyle << " CYCLE " << atts.GetCycle() << endl;
    if(atts.TimeIsAccurate())
        ofile << commentStyle << " TIME " << atts.GetTime() << endl;

    ofile << commentStyle << stem.c_str() << endl;

    // vtkRectilinearGrid *rgrid = vtkRectilinearGrid::SafeDownCast(ds);
    vtkUnstructuredGrid *rgrid = vtkUnstructuredGrid::SafeDownCast(ds);

    vtkPoints *lpts = rgrid->GetPoints() ;
    int npoints = lpts->GetNumberOfPoints();

    for(int i=0; i<npoints; ++i)
    {
        double * positions = lpts->GetPoint(i) ;
        ofile << positions[0] << " " << positions[1] << " " << positions[2] << " " << endl;
    }
}


// ****************************************************************************
//  Method: avtCurve3DWriter::CloseFile
//
//  Purpose:
//      Closes the file.  This does nothing in this case.
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Jul 26, 2012 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
// ****************************************************************************

void
avtCurve3DWriter::CloseFile(void)
{
    // CLOSE FILES
}
