/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the aRAWe  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the aRAWe copyright notice,
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
//                               avtRAWWriter.C                              //
// ************************************************************************* //
    
#include <avtRAWWriter.h>

#include <vtkCellArray.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#ifndef MDSERVER
#include <vtkRectilinearGridFacelistFilter.h>
#include <vtkStructuredGridFacelistFilter.h>
#include <vtkUnstructuredGridFacelistFilter.h>
#endif

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <snprintf.h>
#include <InvalidDBTypeException.h>
#include <DebugStream.h>

// ****************************************************************************
// Method: avtRAWWriter::avtRAWWriter
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 29 16:05:37 PST 2007
//
// Modifications:
//   
// ****************************************************************************

avtRAWWriter::avtRAWWriter()
{
    file = 0;
    nDomains = 0;
}

// ****************************************************************************
// Method: avtRAWWriter::~avtRAWWriter
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 29 16:05:37 PST 2007
//
// Modifications:
//   
// ****************************************************************************

avtRAWWriter::~avtRAWWriter()
{
    CloseFile();
}

// ****************************************************************************
//  Method: avtRAWWriter::OpenFile
//
//  Purpose:
//      Does no actual work.  Just records the stem name for the files.
//
//  Programmer: Brad Whitlock
//  Creation:   September 28, 2007
//
//  Modifications:
//
// ****************************************************************************

void
avtRAWWriter::OpenFile(const std::string &stemname, int nb)
{
#ifndef MDSERVER
    char filename[1024];
    SNPRINTF(filename, 1024, "%s.raw", stemname.c_str());
    file = fopen(filename, "wt");
    if(file == 0)
    {
        EXCEPTION1(InvalidDBTypeException, 
                   "The RAW writer could not open the output file.");
    }
    nDomains = nb;
#endif
}

// ****************************************************************************
//  Method: avtRAWWriter::WriteHeaders
//
//  Purpose:
//      Writes out the RAW header file.
//
//  Programmer: Brad Whitlock
//  Creation:   September 28, 2007
//
//  Modifications:
//
// ****************************************************************************

void
avtRAWWriter::WriteHeaders(const avtDatabaseMetaData *md,
                           std::vector<std::string> &scalars, std::vector<std::string> &vectors,
                           std::vector<std::string> &materials)
{
#ifndef MDSERVER
    bool warn = (scalars.size() > 0) || (vectors.size() > 0) || (materials.size() > 0);

    if(warn)
    {
        avtCallback::IssueWarning("The RAW writer can only save the mesh. Any "
            "scalars, vector, or materials will not be exported");
    }
#endif
}

// ****************************************************************************
//  Method: avtRAWWriter::WriteChunk
//
//  Purpose:
//      This writes out one chunk of an avtDataset.
//
//  Programmer: Brad Whitlock
//  Creation:   September 28, 2007
//
//  Modifications:
//
// ****************************************************************************

void
avtRAWWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
#ifndef MDSERVER
    const char *mName = "avtRAWWriter::WriteChunk: ";

    // Convert non polydata into polydata faces.
    vtkPolyData *pd = 0;
    if(ds->GetDataObjectType() == VTK_POLY_DATA)
    {
        pd = (vtkPolyData *)ds;
        pd->Register(NULL);
    }
    else if(ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGridFacelistFilter *flf = 
            vtkRectilinearGridFacelistFilter::New();
        flf->SetInput((vtkRectilinearGrid *)ds);
        flf->Update();
        pd = flf->GetOutput();
        pd->Register(NULL);
        flf->Delete();        
    }
    else if(ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGridFacelistFilter *flf = 
            vtkStructuredGridFacelistFilter::New();
        flf->SetInput((vtkStructuredGrid *)ds);
        flf->Update();
        pd = flf->GetOutput();
        pd->Register(NULL);
        flf->Delete();
    }
    else if(ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGridFacelistFilter *flf = 
            vtkUnstructuredGridFacelistFilter::New();
        flf->SetInput((vtkUnstructuredGrid *)ds);
        flf->Update();
        pd = flf->GetOutput();
        pd->Register(NULL);
        flf->Delete();
    }
    else
    {
        EXCEPTION1(InvalidDBTypeException, 
                   "The RAW writer can only handle polydata or rectilinear, "
                   "stuctured, unstructured grids.");
    }

    // Now that we have polydata and an open output file, traverse the cells
    // in the polydata and write out the items that can be represented as
    // triangles.
    if(nDomains > 1)
        fprintf(file, "Object%d\n", chunk + 1);

    pd->GetPolys()->InitTraversal();
    vtkIdType nids, *ids = 0;
    while(pd->GetPolys()->GetNextCell(nids, ids))
    {
        if(nids == 3)
        {
            float *ptr = (float *)pd->GetPoints()->GetVoidPointer(0);
            float *A = ptr + 3 * ids[0];
            float *B = ptr + 3 * ids[1];
            float *C = ptr + 3 * ids[2];
            fprintf(file, "%2.6f %2.6f %2.6f %2.6f %2.6f %2.6f %2.6f %2.6f %2.6f\n",
                A[0], A[1], A[2], B[0], B[1], B[2], C[0], C[1], C[2]);
        }
        else if(nids == 4)
        {
            float *ptr = (float *)pd->GetPoints()->GetVoidPointer(0);
            float *A = ptr + 3 * ids[0];
            float *B = ptr + 3 * ids[1];
            float *C = ptr + 3 * ids[2];
            float *D = ptr + 3 * ids[3];
            fprintf(file, "%2.6f %2.6f %2.6f %2.6f %2.6f %2.6f %2.6f %2.6f %2.6f\n",
                A[0], A[1], A[2], B[0], B[1], B[2], C[0], C[1], C[2]);
            fprintf(file, "%2.6f %2.6f %2.6f %2.6f %2.6f %2.6f %2.6f %2.6f %2.6f\n",
                A[0], A[1], A[2], C[0], C[1], C[2], D[0], D[1], D[2]);
        }
        else
            debug5 << mName << nids << " is an unsupported number of vertices" << endl;
    }
    pd->Delete();
#endif
}

// ****************************************************************************
//  Method: avtRAWWriter::CloseFile
//
//  Purpose:
//      Closes the file.  This does nothing in this case.
//
//  Programmer: Brad Whitlock
//  Creation:   September 28, 2007
//
// ****************************************************************************

void
avtRAWWriter::CloseFile(void)
{
#ifndef MDSERVER
    if(file != 0)
    {
        fclose(file);
        file = 0;
    }
#endif
}


