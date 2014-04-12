/*****************************************************************************
* Copyright (c) 2014 Intelligent Light. All rights reserved.
* Work partially supported by DOE Grant SC0007548.
* FieldView XDB Export is provided expressly for use within VisIt.
* All other uses are strictly forbidden.
*****************************************************************************/
#include <avtXDBWriter.h>
#include <avtXDBWriterInternal.h>

#ifdef PARALLEL
#include <avtParallel.h>
#endif

// ****************************************************************************
// Method: avtXDBWriter constructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 14 17:29:14 PST 2014
//
// Modifications:
//
// ****************************************************************************

avtXDBWriter::avtXDBWriter() : avtDatabaseWriter()
{
    int rank = 0;
#ifdef PARALLEL
    rank = PAR_Rank();
#endif
    impl = new avtXDBWriterInternal(rank);
}

// ****************************************************************************
// Method: avtXDBWriter::~avtXDBWriter
//
// Purpose:
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 15:08:07 PST 2014
//
// Modifications:
//
// ****************************************************************************

avtXDBWriter::~avtXDBWriter()
{
    delete impl;
}

// ****************************************************************************
// Method: avtXDBWriter::GetDefaultVariables
//
// Purpose:
//   Create the list of variables that will be used for "default".
//
// Arguments:
//   ds : The data request that contains the variables we're reading.
//
// Returns:    A vector of string variable names that we'll substitute for
//             the "default" variable.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 14 17:29:14 PST 2014
//
// Modifications:
//
// ****************************************************************************

std::vector<std::string>
avtXDBWriter::GetDefaultVariables(avtDataRequest_p ds)
{
    return impl->GetDefaultVariables(GetInput(), ds);
}

// ****************************************************************************
// Method: avtXDBWriter::CheckCompatibility
//
// Purpose:
//   Look at the plot type and the data attributes to determine whether we
//   should even start exporting.
//
// Arguments:
//   plotName : Then name of the plot type being exported.
//
// Returns:    
//
// Note:       Throw an exception if there is a problem that would prevent
//             the writer from exporting the data.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 16:12:27 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
avtXDBWriter::CheckCompatibility(const std::string &plotName)
{
    impl->CheckCompatibility(GetInput(), plotName);
}

// ****************************************************************************
// Method: avtXDBWriter::GetCombineMode
//
// Purpose:
//   Returns what type of geometry combination we want for the plot.
//
// Returns: The type of geometry combination that we want for the plot.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 15:10:05 PST 2014
//
// Modifications:
//
// ****************************************************************************

avtDatabaseWriter::CombineMode
avtXDBWriter::GetCombineMode(const std::string &plotName) const
{
    int mode = impl->GetCombineMode(GetInput(), plotName);
    if(mode == avtXDBWriterInternal::CombineNone)
        return CombineNone;
    else if(mode == avtXDBWriterInternal::CombineNoneGather)
        return CombineNoneGather;
    else if(mode == avtXDBWriterInternal::CombineLike)
        return CombineLike;
    else if(mode == avtXDBWriterInternal::CombineAll)
        return CombineAll;
    return CombineNone;
}

// ****************************************************************************
// Method: avtXDBWriter::CreateNormals
//
// Purpose:
//   Ensure that when we make polydata for the exported geometry that it has
//   normals
//
// Returns:    True because we want normals.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar  5 15:54:58 PST 2014
//
// Modifications:
//
// ****************************************************************************

bool
avtXDBWriter::CreateNormals() const
{
    return impl->CreateNormals(GetInput());
}

// ****************************************************************************
// Method: avtXDBWriter::ApplyMaterialsToContract
//
// Purpose:
//   This method lets a writer change the contract in response to its material
//   needs.
//
// Arguments:
//   c0       : The initial version of the contract.
//   meshname : The name of the mesh for the pipeline.
//   mats     : The material object names.
//   changed  : Return a value indicating whether the contract changed due to
//              calling this method.
//
// Returns:    A new version of the contract.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 14 16:05:50 PDT 2014
//
// Modifications:
//
// ****************************************************************************

avtContract_p
avtXDBWriter::ApplyMaterialsToContract(avtContract_p c0, 
    const std::string &meshname, const std::vector<std::string> &mats,
    bool &changed)
{
    return impl->ApplyMaterialsToContract(GetInput(),
                                          c0, meshname, mats, changed, 
                                          hasMaterialsInProblem);
}

// ****************************************************************************
// Method: avtXDBWriter::GetMaterials
//
// Purpose:
//   Get the materials for pipeline's mesh.
//
// Arguments:
//   needsExecute : True if we know that the pipeline will already need to be
//                  executed again to get the data we need.
//   meshname     : The name of the mesh whose materials we're considering.
//   md           : The database metadata.
//   materialList : The material list that we make.
//
// Returns:    
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 14 14:54:09 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
avtXDBWriter::GetMaterials(bool needsExecute, const std::string &meshname,
    const avtDatabaseMetaData *md,
    std::vector<std::string> &materialList)
{
    impl->GetMaterials(GetInput(), needsExecute, meshname, md, materialList);
}

// ****************************************************************************
// Method: avtXDBWriter::CanHandleMaterials
//
// Purpose:
//   Returns that this writer can deal with materials.
//
// Returns:    True
//
// Note:       This method is used in the base class when deciding to handle
//             materials.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 14 17:28:06 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
avtXDBWriter::CanHandleMaterials(void)
{
    return impl->CanHandleMaterials(GetInput());
}

// ****************************************************************************
// Method: avtXDBWriter::OpenFile
//
// Purpose:
//   Open the file that we want to create.
//
// Arguments:
//   fn        : The name of the file to open for the exported data.
//   numblocks : The number of domains we'll store in the file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 14 17:29:14 PST 2014
//
// Modifications:
//
// ****************************************************************************

void
avtXDBWriter::OpenFile(const std::string &fn, int numblocks)
{
    impl->OpenFile(GetInput(), fn, numblocks);
}

// ****************************************************************************
// Method: avtXDBWriter::WriteHeaders
//
// Purpose:
//   Set the title and notes in the XDB file.
//
// Arguments:
//   md      : The metadata object for the database that we're exporting.
//   scalars : The list of scalars that we're exporting.
//   vectors : The list of vectors that we're exporting.
//   materials : The list of materials that we're exporting.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 14 17:29:14 PST 2014
//
// Modifications:
//
// ****************************************************************************

void
avtXDBWriter::WriteHeaders(const avtDatabaseMetaData *md,
                           std::vector<std::string> &scalars, 
                           std::vector<std::string> &vectors,
                           std::vector<std::string> &materials)
{
    impl->WriteHeaders(GetInput(), md, scalars, vectors, materials);
}

// ****************************************************************************
// Method: avtXDBWriter::BeginPlot
//
// Purpose:
//   This method is called when we begin writing data for a plot. This lets us
//   know the name of the plot we're writing to the file.
//
// Arguments:
//   pn : The name of the plot that we'll write.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 15:09:23 PST 2014
//
// Modifications:
//
// ****************************************************************************

void
avtXDBWriter::BeginPlot(const std::string &pn)
{
    impl->BeginPlot(GetInput(), pn);
}

// ****************************************************************************
// Method: avtXDBWriter::WriteChunk
//
// Purpose:
//   This writes out one chunk of an avtDataset.
//
// Arguments:
//   ds    : The dataset that we're writing out for this chunk of the export.
//   chunk : The number of the chunk we're writing.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 14 17:29:14 PST 2014
//
// Modifications:
//
// ****************************************************************************

void
avtXDBWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    impl->WriteChunk(GetInput(), ds, chunk);
}

// ****************************************************************************
// Method: avtXDBWriter::CloseFile
//
// Purpose:
//   Closes the XDB file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 14 17:29:14 PST 2014
//
// Modifications:
//
// ****************************************************************************

void
avtXDBWriter::CloseFile(void)
{
    impl->CloseFile(GetInput());
}
