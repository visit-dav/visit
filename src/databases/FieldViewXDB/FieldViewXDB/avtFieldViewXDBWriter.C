/*****************************************************************************
* Copyright (c) 2014 Intelligent Light. All rights reserved.
* Work partially supported by DOE Grant SC0007548.
* FieldView XDB Export is provided expressly for use within VisIt.
* All other uses are strictly forbidden.
*****************************************************************************/
#include <avtFieldViewXDBWriter.h>
#include <avtFieldViewXDBWriterInternal.h>

#ifdef PARALLEL
#include <avtParallel.h>
#endif

// ****************************************************************************
// Method: avtFieldViewXDBWriter constructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 14 17:29:14 PST 2014
//
// Modifications:
//
// ****************************************************************************

avtFieldViewXDBWriter::avtFieldViewXDBWriter() : avtDatabaseWriter()
{
    int rank = 0;
#ifdef PARALLEL
    rank = PAR_Rank();
#endif
    impl = new avtFieldViewXDBWriterInternal(rank);
}

// ****************************************************************************
// Method: avtFieldViewXDBWriter::~avtFieldViewXDBWriter
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

avtFieldViewXDBWriter::~avtFieldViewXDBWriter()
{
    delete impl;
}

// ****************************************************************************
// Method: avtFieldViewXDBWriter::GetDefaultVariables
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
avtFieldViewXDBWriter::GetDefaultVariables(avtDataRequest_p ds)
{
    return impl->GetDefaultVariables(GetInput(), ds);
}

// ****************************************************************************
// Method: avtFieldViewXDBWriter::CheckCompatibility
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
avtFieldViewXDBWriter::CheckCompatibility(const std::string &plotName)
{
    impl->CheckCompatibility(GetInput(), plotName);
}

// ****************************************************************************
// Method: avtFieldViewXDBWriter::GetCombineMode
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
avtFieldViewXDBWriter::GetCombineMode(const std::string &plotName) const
{
    int mode = impl->GetCombineMode(GetInput(), plotName);
    if(mode == avtFieldViewXDBWriterInternal::CombineNone)
        return CombineNone;
    else if(mode == avtFieldViewXDBWriterInternal::CombineNoneGather)
        return CombineNoneGather;
    else if(mode == avtFieldViewXDBWriterInternal::CombineLike)
        return CombineLike;
    else if(mode == avtFieldViewXDBWriterInternal::CombineAll)
        return CombineAll;
    return CombineNone;
}

// ****************************************************************************
// Method: avtFieldViewXDBWriter::CreateNormals
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
avtFieldViewXDBWriter::CreateNormals() const
{
    return impl->CreateNormals(GetInput());
}

// ****************************************************************************
// Method: avtFieldViewXDBWriter::ApplyMaterialsToContract
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
avtFieldViewXDBWriter::ApplyMaterialsToContract(avtContract_p c0, 
    const std::string &meshname, const std::vector<std::string> &mats,
    bool &changed)
{
    return impl->ApplyMaterialsToContract(GetInput(),
                                          c0, meshname, mats, changed, 
                                          hasMaterialsInProblem);
}

// ****************************************************************************
// Method: avtFieldViewXDBWriter::GetMaterials
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
avtFieldViewXDBWriter::GetMaterials(bool needsExecute, const std::string &meshname,
    const avtDatabaseMetaData *md,
    std::vector<std::string> &materialList)
{
    impl->GetMaterials(GetInput(), needsExecute, meshname, md, materialList);
}

// ****************************************************************************
// Method: avtFieldViewXDBWriter::CanHandleMaterials
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
avtFieldViewXDBWriter::CanHandleMaterials(void)
{
    return impl->CanHandleMaterials(GetInput());
}

// ****************************************************************************
// Method: avtFieldViewXDBWriter::OpenFile
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
avtFieldViewXDBWriter::OpenFile(const std::string &fn, int numblocks)
{
    impl->OpenFile(GetInput(), fn, numblocks);
}

// ****************************************************************************
// Method: avtFieldViewXDBWriter::WriteHeaders
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
avtFieldViewXDBWriter::WriteHeaders(const avtDatabaseMetaData *md,
                           std::vector<std::string> &scalars, 
                           std::vector<std::string> &vectors,
                           std::vector<std::string> &materials)
{
    impl->WriteHeaders(GetInput(), md, scalars, vectors, materials);
}

// ****************************************************************************
// Method: avtFieldViewXDBWriter::BeginPlot
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
avtFieldViewXDBWriter::BeginPlot(const std::string &pn)
{
    impl->BeginPlot(GetInput(), pn);
}

// ****************************************************************************
// Method: avtFieldViewXDBWriter::WriteChunk
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
avtFieldViewXDBWriter::WriteChunk(vtkDataSet *ds, int chunk)
{
    impl->WriteChunk(GetInput(), ds, chunk);
}

// ****************************************************************************
// Method: avtFieldViewXDBWriter::CloseFile
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
avtFieldViewXDBWriter::CloseFile(void)
{
    impl->CloseFile(GetInput());
}
