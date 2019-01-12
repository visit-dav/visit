/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
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
        SNPRINTF(name, 1024, "%s.curve", stem.c_str());
    else
        SNPRINTF(name, 1024, "%s.%03d.curve", stem.c_str(), chunk);

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
