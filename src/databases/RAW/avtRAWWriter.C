// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtRAWWriter.C                              //
// ************************************************************************* //

#include <avtRAWWriter.h>

#include <visit-config.h> // For LIB_VERSION_GE
#include <vtkCellArray.h>
#if LIB_VERSION_GE(VTK,9,1,0)
#include <vtkCellArrayIterator.h>
#endif
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#ifndef MDSERVER
#include <vtkRectilinearGridFacelistFilter.h>
#include <vtkStructuredGridFacelistFilter.h>
#include <vtkUnstructuredGridFacelistFilter.h>
#endif

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
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
//      Open the file for output.
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
    if(writeContext.Rank() == 0)
    {
        char filename[1024];
        if(writeContext.GroupSize() > 1)
            snprintf(filename, 1024, "%s.%d.raw", stemname.c_str(), writeContext.GroupRank());
        else
            snprintf(filename, 1024, "%s.raw", stemname.c_str());
        file = fopen(filename, "wt");
        if(file == 0)
        {
            EXCEPTION1(InvalidDBTypeException,
                       "The RAW writer could not open the output file.");
        }
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
                           const std::vector<std::string> &scalars,
                           const std::vector<std::string> &vectors,
                           const std::vector<std::string> &materials)
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
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK9: use vtkCellArrayIterator.
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
        flf->SetInputData((vtkRectilinearGrid *)ds);
        flf->Update();
        pd = flf->GetOutput();
        pd->Register(NULL);
        flf->Delete();
    }
    else if(ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGridFacelistFilter *flf =
            vtkStructuredGridFacelistFilter::New();
        flf->SetInputData((vtkStructuredGrid *)ds);
        flf->Update();
        pd = flf->GetOutput();
        pd->Register(NULL);
        flf->Delete();
    }
    else if(ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
        vtkUnstructuredGridFacelistFilter *flf =
            vtkUnstructuredGridFacelistFilter::New();
        flf->SetInputData((vtkUnstructuredGrid *)ds);
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

    vtkIdType nids;
#if LIB_VERSION_LE(VTK,8,1,0)
    pd->GetPolys()->InitTraversal();
    vtkIdType *ids = 0;
    while(pd->GetPolys()->GetNextCell(nids, ids))
    {
#else
    const vtkIdType *ids = nullptr;
    auto iter = vtk::TakeSmartPointer(pd->GetPolys()->NewIterator());
    for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell())
    {
        iter->GetCurrentCell(nids, ids);
#endif
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

// ****************************************************************************
// Method: avtRAWWriter::CreateTrianglePolyData
//
// Purpose:
//   Tell VisIt's export that we'll want triangles.
//
// Returns:    True
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep  8 17:00:23 PDT 2015
//
// Modifications:
//
// ****************************************************************************

bool
avtRAWWriter::CreateTrianglePolyData() const
{
    return true;
}

// ****************************************************************************
//  Method: avtRAWWriter::GetCombineMode
//
//  Purpose:
//     Provides a hint to the export mechanism to tell it how to combine data.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Sep  8 15:36:45 PDT 2015
//
// ****************************************************************************

avtDatabaseWriter::CombineMode
avtRAWWriter::GetCombineMode(const std::string &) const
{
    return CombineAll;
}


