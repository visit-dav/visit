// ************************************************************************* //
//                            avtGenericDatabase.C                           //
// ************************************************************************* //

#include <avtGenericDatabase.h>

#include <float.h>

#include <string>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <vtkCellData.h>
#include <vtkCellLocator.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridBoundaryFilter.h>
#include <vtkVisItUtility.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtDatasetCollection.h>
#include <avtDomainBoundaries.h>
#include <avtDomainNesting.h>
#include <avtFileFormatInterface.h>
#include <avtMetaData.h>
#include <avtMixedVariable.h>
#include <avtParallel.h>
#include <avtSILGenerator.h>
#include <avtSILRestrictionTraverser.h>
#include <avtSourceFromDatabase.h>
#include <avtTypes.h>
#include <PickAttributes.h>
#include <PickVarInfo.h>
#include <TetMIR.h>
#include <ZooMIR.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <NoInputException.h>
#include <TimingsManager.h>


using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtGenericDatabase constructor
//
//  Arguments:
//      inter   The interface -- the database will delete this object when it
//              is done with it.
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2001
// 
//  Modifications:
//
//    Hank Childs, Thu Sep 20 14:19:34 PDT 2001
//    Give the formats a chance to put stuff in the cache.
//
// ****************************************************************************

avtGenericDatabase::avtGenericDatabase(avtFileFormatInterface *inter)
{
    Interface = inter;
    Interface->SetCache(&cache);
    lastTimestep = -1;
}


// ****************************************************************************
//  Method: avtGenericDatabase destructor
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2001
//
// ****************************************************************************

avtGenericDatabase::~avtGenericDatabase()
{
    if (Interface != NULL)
    {
        delete Interface;
    }
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetFilename
//
//  Purpose:
//      Gets the name of the database.
//
//  Returns:    The name for the database.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
// ****************************************************************************

const char *
avtGenericDatabase::GetFilename(int ts)
{
    return Interface->GetFilename(ts);
}


// ****************************************************************************
//  Method: avtGenericDatabase::SetDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data using the file format interface.
//
//  Arguments:
//      md        : The meta-data to set.
//      timeState : The time state that we're interested in.
//
//  Programmer: Hank Childs
//  Creation:   March 2, 2001
//
//  Modifications:
//    Brad Whitlock, Wed May 14 09:15:18 PDT 2003
//    I added the optional timeState argument.
//
// ****************************************************************************

void
avtGenericDatabase::SetDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    Interface->SetDatabaseMetaData(md, timeState);
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetOutput
//
//  Purpose:
//      Takes a database specification and generates the correct dataset
//      output.
//
//  Arguments:
//      spec    A database specification.
//      v       An object that represents the validity of the dataset.
//
//  Returns:    A domain tree with a dataset for each domain.
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Apr  9 14:47:12 PDT 2001
//    Made this method return avtDataTree_p.
//
//    Hank Childs, Tue May 22 10:47:31 PDT 2001
//    Use a database specification instead of a SIL restriction.
//
//    Hank Childs, Tue Jul 24 14:17:14 PDT 2001
//    Add data validity argument.
//
//    Hank Childs, Fri Oct  5 15:30:57 PDT 2001
//    Account for file formats that do their own material selection.
//
//    Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001 
//    Create domain labels for output data tree when requested. 
//
//    Hank Childs, Tue Oct 23 09:18:43 PDT 2001
//    Add support for reading in multiple variables.
//
//    Eric Brugger, Mon Nov  5 13:32:49 PST 2001
//    Modified to always compile the timing code.
//
//    Jeremy Meredith, Wed Nov 21 15:11:45 PST 2001
//    Disabled ghost zone communication temporarily.
//
//    Hank Childs, Wed Nov 28 13:01:00 PST 2001
//    Do not communicate ghost zones if they are not appropriate.
//
//    Kathleen Bonnell,  Fri Nov 30 10:36:41 PST 2001
//    Create Original Zones array if data spec specifies they are needed.
//
//    Jeremy Meredith, Thu Dec 13 11:42:01 PST 2001
//    Re-enabled ghost zone communication again.
//
//    Hank Childs, Wed Dec 19 09:37:04 PST 2001
//    Added support for species selection.
//
//    Kathleen Bonnell, Mon May 20 17:01:31 PDT 2002  
//    Added 'domains' parameter to CreateOriginalZones. 
//
//    Hank Childs, Mon Sep 30 17:56:43 PDT 2002
//    Add support for creating structured indices.
//
//    Hank Childs, Fri Nov 22 16:39:13 PST 2002
//    Use the SIL restriction traverser since SIL restriction routines were
//    antiquated.
//
//    Kathleen Bonnell, Fri Dec  6 12:10:04 PST 2002   
//    Add OriginalCellsArray whenever MatSelect will be performed,
//    to ensure that pick will operate correctly.  
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003
//    Tell the MetaData when the originalCells array has been added.
// 
//    Jeremy Meredith, Thu Jun 12 09:06:49 PDT 2003
//    Added the data spec to the input of PopulateDataObjectInformation.
//
//    Jeremy Meredith, Sat Aug  2 20:40:04 PDT 2003
//    If any processor decides to do material selection, they all should.
//    Unified shouldDoMatSelect across all processors.
//
// ****************************************************************************

avtDataTree_p
avtGenericDatabase::GetOutput(avtDataSpecification_p spec,
                              avtSourceFromDatabase *src)
{
    int timerHandle = visitTimer->StartTimer();
    int timeStep = spec->GetTimestep();

    UpdateInternalState(timeStep);

    //
    // Determine which domains the SIL restriction has turned on.
    //
    avtSILRestriction_p silr = spec->GetRestriction();
    avtSILRestrictionTraverser trav(silr);
    vector<int> domains, allDomains;
    trav.GetDomainList(domains);
    trav.GetDomainListAllProcs(allDomains);


    //
    // Set up a data tree for each of the domains.
    //
    int                    nDomains = domains.size();
    avtDatasetCollection   datasetCollection(nDomains);

    //
    // This is the primary routine that reads things in from disk.
    //
    ReadDataset(datasetCollection, domains, spec, src);

    //
    // Do species selection if appropriate.
    //
    vector<bool> speciesList;
    if (trav.GetSpecies(speciesList))
    {
        SpeciesSelect(datasetCollection, domains, speciesList, spec, src);
    }


    //
    //  HACK!!! Pick requires original cells array whenever MaterialSelection
    //  has occurred.  Rather than incur the expense of re-execution at StartPick,
    //  always send the array when MatSelected, until a better solution is derived.
    // 
    bool shouldDoMatSelect = false;
    for (int i = 0 ; i < datasetCollection.GetNDomains() ; i++)
    {
        shouldDoMatSelect = shouldDoMatSelect || datasetCollection.needsMatSelect[i];
    }

#ifdef PARALLEL
    //
    // If any processor decides to do material selection, they all should
    //
    int shouldDoMatSelectAsInt = (shouldDoMatSelect ? 1 : 0);
    int shouldDoMatSelectAsIntCollective;
    MPI_Allreduce(&shouldDoMatSelectAsInt, &shouldDoMatSelectAsIntCollective,
                  1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    shouldDoMatSelect = bool(shouldDoMatSelectAsIntCollective);
#endif

    if (shouldDoMatSelect)
    {
        spec->TurnZoneNumbersOn();
    }

    //
    // Add node numbers if requested.
    //
    if (spec->NeedNodeNumbers())
    {
        CreateOriginalNodes(datasetCollection, domains, src);
    }

    //
    // Add zone numbers if requested.
    //
    if (spec->NeedZoneNumbers())
    {
        CreateOriginalZones(datasetCollection, domains, src);
        //
        // Tell everything downstream that we do have original cells.
        //
        avtDatabaseMetaData *md = GetMetaData(timeStep);
        string meshname = md->MeshForVar(spec->GetVariable());
        GetMetaData(timeStep)->SetContainsOriginalCells(meshname, true);
    }

    //
    // Add the indices for a structured mesh if requested.
    //
    if (spec->NeedStructuredIndices())
    {
        CreateStructuredIndices(datasetCollection, src);
        //
        // Tell everything downstream that we do have original cells.
        //
        avtDatabaseMetaData *md = GetMetaData(timeStep);
        string meshname = md->MeshForVar(spec->GetVariable());
        GetMetaData(timeStep)->SetContainsOriginalCells(meshname, true);
    }

    //
    // Communicates ghost zones if they are not present and we have domain
    // boundary information.
    //
    bool didGhosts = false;
    if (spec->GhostZonesAreAppropriate())
    {
        didGhosts = CommunicateGhosts(datasetCollection, domains, spec, src);
    }

    //
    // Apply ghosting when domains nest within other domains (AMR meshes)
    //
    bool didNestingGhosts =
        ApplyGhostForDomainNesting(datasetCollection, domains, allDomains, spec);

    //
    // Finally, do the material selection.
    //
    MaterialSelect(datasetCollection, domains, spec, src, didGhosts);

    //
    // Now make something that AVT will understand downstream.
    //
    avtDataTree_p rv = datasetCollection.AssembleDataTree(domains);
  
    char str[1024];
    sprintf(str, "Getting dataset for %s", spec->GetVariable());
    visitTimer->StopTimer(timerHandle, str);
    visitTimer->DumpTimings();

    //
    // As we have actually read the data, our opinion on what the data looked
    // like may have changed slightly (we may have decided it had ghost zones
    // for example), so call this again.
    //
    avtDataObject_p dob = src->GetOutput();
    PopulateDataObjectInformation(dob, spec->GetVariable(), timeStep, *spec);

    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::UpdateInternalState
//
//  Purpose:
//      Do some internal bookkeeping.  This is mostly to handle current
//      timestep information.
//
//  Arguments:
//      ts      The incoming timestep.
//
//  Programmer: Hank Childs
//  Creation:   October 26, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Jan  7 17:34:00 PST 2002 
//    Clear out the cache when the timestep changes.
//
// ****************************************************************************

void
avtGenericDatabase::UpdateInternalState(int ts)
{
    //
    // We will run out of file descriptors if we are not careful!
    //
    if (ts != lastTimestep)
    {
        if (lastTimestep != -1)
        {
            int allDomains = -1;
            debug4 << "Generic database freeing up resources "
                   << " (file descriptors, memory) for timestep "
                   << lastTimestep << endl;
            Interface->FreeUpResources(lastTimestep, allDomains);
            cache.ClearTimestep(lastTimestep);
        }
    }
    lastTimestep = ts;
}


// ****************************************************************************
//  Method: avtGenericDatabase::FreeUpResources
//
//  Purpose:
//      Release any system resources that we're using.
//
//  Programmer: Sean Ahern
//  Creation:   Tue May 21 12:00:49 PDT 2002
//
//  Modifications:
//
// ****************************************************************************
void
avtGenericDatabase::FreeUpResources(void)
{
    debug4 << "Generic database freeing up resources (file descriptors, "
           << "memory)" << endl;
    Interface->FreeUpResources(-1, -1);
    cache.ClearTimestep(lastTimestep);
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetDataset
//
//  Purpose:
//      Determines what the dataset type is requested (scalar var, material,
//      etc.) and calls the appropriate routine.
//
//  Arguments:
//      varname      The variable for that domain.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      matname      The material of interest (may be _all).
//      vars2nd      The list of secondary variables.
//      src          The source from a database.
//
//  Returns:         The dataset for that block.
//
//  Programmer:  Hank Childs
//  Creation:    November 7, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  1 09:20:52 PST 2001
//    Moved from avtDatabase to avtGenericDatabase.
//
//    Hank Childs, Mon Mar 19 16:35:56 PST 2001
//    Added logic for vector datasets.
//
//    Hank Childs, Tue Mar 27 08:59:23 PST 2001 
//    Added logic for mesh datasets.
//
//    Hank Childs, Fri Oct  5 15:52:13 PDT 2001
//    Added a argument for the database source.  Also an argument for the
//    material name.
//
//    Hank Childs, Tue Oct 23 09:18:43 PDT 2001
//    Added support for secondary variables.
//
//    Hank Childs, Mon Nov  5 16:54:18 PST 2001
//    Extend support for secondary variables to include vectors.
//
//    Hank Childs, Fri Aug  1 21:49:01 PDT 2003
//    Treat curves like meshes.
//
//    Hank Childs, Mon Sep 22 07:48:34 PDT 2003
//    Added support for tensors.
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetDataset(const char *varname, int ts, int domain,
                        const char *matname, const vector<CharStrRef> &vars2nd, 
                        avtSourceFromDatabase *src)
{
    vtkDataSet *rv = NULL;
    avtVarType type = GetMetaData(ts)->DetermineVarType(varname);

    if (strcmp(matname, "_all") == 0)
    {
        Interface->TurnMaterialSelectionOff();
    }
    else
    {
        Interface->TurnMaterialSelectionOn(matname);
    }

    switch (type)
    {
      case AVT_SCALAR_VAR:
        rv = GetScalarVarDataset(varname, ts, domain, matname);
        break;

      case AVT_VECTOR_VAR:
        rv = GetVectorVarDataset(varname, ts, domain, matname);
        break;

      case AVT_TENSOR_VAR:
        rv = GetTensorVarDataset(varname, ts, domain, matname);
        break;

      case AVT_SYMMETRIC_TENSOR_VAR:
        rv = GetSymmetricTensorVarDataset(varname, ts, domain, matname);
        break;

      case AVT_MATERIAL:
        rv = GetMaterialDataset(varname, ts, domain, matname);
        break;

      case AVT_MESH:
      case AVT_CURVE:
        rv = GetMeshDataset(varname, ts, domain, matname);
        break;

      case AVT_MATSPECIES:
        rv = GetSpeciesDataset(varname, ts, domain, matname);
        break;

      default:
        EXCEPTION1(InvalidVariableException, varname);
    }

    if (rv != NULL && vars2nd.size() > 0)
    {
        AddSecondaryVariables(rv, ts, domain, matname, vars2nd);
    }

    Interface->TurnMaterialSelectionOff();

    //
    // Make sure that every domain we read has its extents merged into the
    // extents for all the dataset.
    //
    src->MergeExtents(rv);

    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetScalarVarDataset
//
//  Purpose:
//      Constructs a dataset for a scalar variable.
//
//  Arguments:
//      varname      The variable for that domain.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  1 09:20:52 PST 2001
//    Moved from avtDatabase to avtGenericDatabase.
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
//    Hank Childs, Thu Oct 11 13:04:25 PDT 2001
//    Added support for NULL meshes.
//
//    Hank Childs, Tue Oct 23 09:18:43 PDT 2001
//    Added support for secondary variables.
//
//    Hank Childs, Thu Oct 25 15:26:08 PDT 2001
//    Allow for file formats that don't have domains.
//
//    Hank Childs, Mon Nov  5 16:55:49 PST 2001
//    Remove support of secondary variables in this routine in favor of a more
//    general routine that supports vectors.
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002 
//    vtkScalars has been deprecated in VTK 4.0.  Use vtkDataArray instead.
//
//    Jeremy Meredith, Wed Mar 19 12:22:20 PST 2003
//    Allow for a NULL var as well as a NULL mesh.
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetScalarVarDataset(const char *varname, int ts,
                                        int domain, const char *material)
{
    const avtScalarMetaData *smd = GetMetaData(ts)->GetScalar(varname);
    if (smd == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    vtkDataArray *var  = GetScalarVariable(varname, ts, domain, material);

    if (var == NULL)
    {
        //
        // Some variables don't have a var for every domain, even if the
        // mesh exists there.  Just propagate the NULL up.  
        //
        return NULL;
    }

    string meshname  = GetMetaData(ts)->MeshForVar(varname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material);

    if (mesh == NULL)
    {
        //
        // Some file formats don't have a mesh for every domain (like Exodus
        // when material selection is applied).  Just propagate the NULL up.  
        //
        return NULL;
    }

    //
    // Set up the scalar var's name in case we have more than one.
    //
    var->SetName(varname);

    if (smd->centering == AVT_NODECENT)
    {
        mesh->GetPointData()->SetScalars(var);
    }
    else
    {
        mesh->GetCellData()->SetScalars(var);
    }

    return mesh;
}


// ****************************************************************************
//  Method: avtGenericDatabase::AddSecondaryVariables
//
//  Purpose:
//      Adds the secondary variables to the dataset.
//
//  Arguments:
//      ds           The vtk dataset to add to.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//      vars2nd      A list of secondary variable to add.
//
//  Programmer: Hank Childs
//  Creation:   November 5, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002
//    vtkScalars has been deprecated in VTK 4.0.  Use vtkDataArray instead.
//
//    Hank Childs, Tue Sep 17 09:50:24 PDT 2002
//    Add support for species variables.
//
//    Hank Childs, Fri Jul 25 10:49:56 PDT 2003
//    Allow for meshes to be specified (then ignored).
//
//    Hank Childs, Thu Aug 21 11:01:22 PDT 2003
//    Allow for materials to be specified (then ignored).
//
// ****************************************************************************

void
avtGenericDatabase::AddSecondaryVariables(vtkDataSet *ds, int ts, int domain,
                  const char *material, const std::vector<CharStrRef> &vars2nd)
{
    int nzones = ds->GetNumberOfCells();

    //
    // If we have any secondary arrays, then fetch those as well.
    //
    int num2ndVars = vars2nd.size();
    for (int i = 0 ; i < num2ndVars ; i++)
    {
        const char *varName = *(vars2nd[i]);
        avtDatabaseMetaData *md = GetMetaData(ts);
        avtVarType vt = md->DetermineVarType(varName);

        //
        // They asked for the mesh as a secondary variable.  Just ignore this
        // request -- it is likely from the expression code.
        //
        if (vt == AVT_MESH || vt == AVT_MATERIAL)
            continue;

        //
        // Do some preparation.  Decide if we have a scalar or a vector and
        // if it is node centered or zone centered.
        //
        vtkDataSetAttributes *atts = NULL;
        switch (vt)
        {
          case AVT_SCALAR_VAR:
            {
                const avtScalarMetaData *smd=GetMetaData(ts)->GetScalar(varName);
                if (smd->centering == AVT_NODECENT)
                {
                    atts = ds->GetPointData();
                }
                else
                {
                    atts = ds->GetCellData();
                }
            }
            break;

          case AVT_VECTOR_VAR:
            {
                const avtVectorMetaData *vmd=GetMetaData(ts)->GetVector(varName);
                if (vmd->centering == AVT_NODECENT)
                {
                    atts = ds->GetPointData();
                }
                else
                {
                    atts = ds->GetCellData();
                }
            }
            break;

          case AVT_MATSPECIES:
            atts = ds->GetCellData();
            break;

          default:
            EXCEPTION1(InvalidVariableException, varName);
        }

        //
        // Okay, now get the scalars or vectors and add them to the dataset.
        //
        vtkDataArray *dat = NULL;
        switch (vt)
        {
          case AVT_SCALAR_VAR:
            dat = GetScalarVariable(varName, ts, domain, material);
            break;
          case AVT_VECTOR_VAR:
            dat = GetVectorVariable(varName, ts, domain, material);
            break;
          case AVT_MATSPECIES:
            dat = GetSpeciesVariable(varName, ts, domain, material, nzones);
            break;
          default:
            EXCEPTION1(InvalidVariableException, varName);
        }
           
        dat->SetName(varName);
        atts->AddArray(dat);
    }
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetMeshDataset
//
//  Purpose:
//      Constructs a dataset for a mesh variable.
//
//  Arguments:
//      varname      The variable for that domain.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 22, 2001 
//
//  Modifications:
// 
//    Kathleen Bonnell, Fri Jun 15 11:34:26 PDT 2001
//    Call method AddOriginalCellsArray.
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
//    Hank Childs, Thu Oct 11 13:04:25 PDT 2001
//    Added support for NULL meshes.
//
//    Kathleen Bonnell, Mon Mar 25 17:30:07 PST 2002     
//    Removed call to AddOriginalCellsArray.  Instead made the
//    mesh filter set the necessary flag in the dataspecification,
//    so the method is only called from one place. 
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetMeshDataset(const char *varname, int ts, int domain,
                                   const char *material)
{
    string meshname  = GetMetaData(ts)->MeshForVar(varname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material);

    return mesh;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetVectorVarDataset
//
//  Purpose:
//      Constructs a dataset for a vector variable.
//
//  Arguments:
//      varname      The variable for that domain.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
//    Hank Childs, Thu Oct 11 13:04:25 PDT 2001
//    Added support for NULL meshes.
//
//    Hank Childs, Thu Jan  3 10:29:02 PST 2002
//    Name the vector so we can reference it later.
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002 
//    vtkVectors has been deprecated in VTK 4.0.  Use vtkDataArray instead.
//
//    Jeremy Meredith, Wed Mar 19 12:22:20 PST 2003
//    Allow for a NULL var as well as a NULL mesh.
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetVectorVarDataset(const char *varname, int ts,
                                        int domain, const char *material)
{
    const avtVectorMetaData *vmd = GetMetaData(ts)->GetVector(varname);
    if (vmd == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    vtkDataArray *var = GetVectorVariable(varname, ts, domain, material);

    if (var == NULL)
    {
        //
        // Some variables don't have a var for every domain, even if the
        // mesh exists there.  Just propagate the NULL up.  
        //
        return NULL;
    }

    string meshname  = GetMetaData(ts)->MeshForVar(varname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material);

    if (mesh == NULL)
    {
        //
        // Some file formats don't have a mesh for every domain (like Exodus
        // when material selection is applied).  Just propagate the NULL up.  
        //
        return NULL;
    }

    //
    // Set up the vector var's name in case we have more than one.
    //
    var->SetName(varname);

    if (vmd->centering == AVT_NODECENT)
    {
        mesh->GetPointData()->SetVectors(var);
    }
    else
    {
        mesh->GetCellData()->SetVectors(var);
    }

    return mesh;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetTensorVarDataset
//
//  Purpose:
//      Constructs a dataset for a tensor variable.
//
//  Arguments:
//      varname      The variable for that domain.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetTensorVarDataset(const char *varname, int ts,
                                        int domain, const char *material)
{
    const avtTensorMetaData *tmd = GetMetaData(ts)->GetTensor(varname);
    if (tmd == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    vtkDataArray *var = GetTensorVariable(varname, ts, domain, material);

    if (var == NULL)
    {
        //
        // Some variables don't have a var for every domain, even if the
        // mesh exists there.  Just propagate the NULL up.  
        //
        return NULL;
    }

    string meshname  = GetMetaData(ts)->MeshForVar(varname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material);

    if (mesh == NULL)
    {
        //
        // Some file formats don't have a mesh for every domain (like Exodus
        // when material selection is applied).  Just propagate the NULL up.  
        //
        return NULL;
    }

    //
    // Set up the tensor var's name in case we have more than one.
    //
    var->SetName(varname);

    if (tmd->centering == AVT_NODECENT)
    {
        mesh->GetPointData()->SetTensors(var);
    }
    else
    {
        mesh->GetCellData()->SetTensors(var);
    }

    return mesh;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetSymmetricTensorVarDataset
//
//  Purpose:
//      Constructs a dataset for a symmetric tensor variable.
//
//  Arguments:
//      varname      The variable for that domain.
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetSymmetricTensorVarDataset(const char *varname, int ts,
                                        int domain, const char *material)
{
    const avtSymmetricTensorMetaData *tmd = 
                                         GetMetaData(ts)->GetSymmTensor(varname);
    if (tmd == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    vtkDataArray *var = GetSymmetricTensorVariable(varname, ts, domain,
                                                   material);

    if (var == NULL)
    {
        //
        // Some variables don't have a var for every domain, even if the
        // mesh exists there.  Just propagate the NULL up.  
        //
        return NULL;
    }

    string meshname  = GetMetaData(ts)->MeshForVar(varname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material);

    if (mesh == NULL)
    {
        //
        // Some file formats don't have a mesh for every domain (like Exodus
        // when material selection is applied).  Just propagate the NULL up.  
        //
        return NULL;
    }

    //
    // Set up the tensor var's name in case we have more than one.
    //
    var->SetName(varname);

    if (tmd->centering == AVT_NODECENT)
    {
        mesh->GetPointData()->SetTensors(var);
    }
    else
    {
        mesh->GetCellData()->SetTensors(var);
    }

    return mesh;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetMaterialDataset
//
//  Purpose:
//      Constructs a dataset for a material.  This is only the mesh, as the
//      material is retrieved through the GetAuxiliaryData mechanism.
//
//  Arguments:
//      matname      The material for that domain (like "mat1").
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  1 09:20:52 PST 2001
//    Moved from avtDatabase to avtGenericDatabase.
//
//    Eric Brugger, Thu Sep 20 10:12:56 PDT 2001
//    I modified the routine to return a non material selected mesh, since
//    that occurs later.
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetMaterialDataset(const char *matname, int ts, int domain,
                                       const char *material)
{
    string meshname  = GetMetaData(ts)->MeshForVar(matname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material);

    return mesh;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetSpeciesDataset
//
//  Purpose:
//      Constructs a dataset for a species.  This is only the mesh, as the
//      species is retrieved through the GetAuxiliaryData mechanism.
//
//  Arguments:
//      specname     The species for that domain (like "spec1").
//      ts           The timestep of interest.
//      domain       The domain of the dataset to retrieve.
//      material     The name of the material we are getting.
//
//  Returns:         The dataset for that block.
//
//  Programmer: Hank Childs and Jeremy Meredith
//  Creation:   December 19, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002
//    vtkScalars has been deprecated in VTK 4.0.  Use vtkDataArray and 
//    vtkFloatArray instead.
//
//    Hank Childs, Thu Jul  4 13:07:23 PDT 2002
//    Add a name to the mixed variable.
//    
//    Hank Childs, Tue Sep 17 09:50:24 PDT 2002
//    Create the species variable in its own subroutine.
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetSpeciesDataset(const char *specname, int ts, int domain,
                                      const char *material)
{
    string meshname  = GetMetaData(ts)->MeshForVar(specname);
    vtkDataSet *mesh = GetMesh(meshname.c_str(), ts, domain, material);

    if (mesh == NULL)
    {
        //
        // Some file formats don't have a mesh for every domain (like Exodus
        // when material selection is applied).  Just propagate the NULL up.  
        //
        return NULL;
    }

    vtkDataArray *spec = GetSpeciesVariable(specname, ts, domain, material,
                                            mesh->GetNumberOfCells());
    mesh->GetCellData()->SetScalars(spec);
    spec->Delete();
    
    return mesh;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetSpeciesVariable
//
//  Purpose:
//      Creates a zonal variable that serves as the species.  Also sets up
//      a mixed variable for use with material selection.
//
//  Arguments:
//      specname    The name of the species.
//      ts          The timestep.
//      domain      The domain number.
//      material    The name of the associated material.
//      nzones      The number of zones in the mesh for this domain.
//
//  Returrns:   An array (of all 1.'s) that is the species.
//
//  Notes:      The calling routine must de-reference the returned array.
//
//  Programmer: Hank Childs
//  Creation:   September 17, 2002
//
// ****************************************************************************

vtkDataArray *
avtGenericDatabase::GetSpeciesVariable(const char *specname, int ts, 
                                  int domain, const char *material, int nzones)
{
    int i;

    vtkFloatArray *allOnes = vtkFloatArray::New();
    allOnes->SetNumberOfTuples(nzones);
    float *ptr = allOnes->GetPointer(0);
    for (i = 0 ; i < nzones ; i++)
    {
        *ptr = 1.;
        ptr++;
    }
    allOnes->SetName(specname);

    //
    // Now create a mixed variable.  We should probably be caching this so we
    // don't generate every time we ask for this dataset.
    //
    avtMaterial *mat = GetMaterial(domain, specname, ts);
    int mixlen = mat->GetMixlen();
    float *mixvarbuff = new float[mixlen];
    for (i = 0 ; i < mixlen ; i++)
        mixvarbuff[i] = 1.;
    avtMixedVariable *mixvar = new avtMixedVariable(mixvarbuff, mixlen, 
                                                    specname);
    delete [] mixvarbuff;
    
    //
    // After this routine is called, the main routine will look for mixed
    // variables in the cache.  So: all we have to do is put this mixed
    // variable in the cache and we can forget about it.
    //
    void_ref_ptr vr = void_ref_ptr(mixvar, avtMixedVariable::Destruct);
    cache.CacheVoidRef(specname, AUXILIARY_DATA_MIXED_VARIABLE, ts, domain,vr);

    return allOnes;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetScalarVariable
//
//  Purpose:
//      Checks to see if a variable is already in cache and fetches it if it
//      is not.
//
//  Arguments:
//      varname    The name of the variable.
//      ts         The timestep for the variable.
//      domain     The domain for the variable.
//      material   The name of the material we are getting.
//
//  Returns:    A vtkScalars for the variable.
//
//  Programmer: Hank Childs
//  Creation:   October 31, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  1 09:20:52 PST 2001
//    Moved from avtDatabase to avtGenericDatabase.
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002 
//    vtkScalars has been deprecated in VTK 4.0.  Use vtkDataArray instead.
//
//    Hank Childs, Fri Mar 14 20:56:40 PST 2003
//    Turned off caching in some instances.
//
// ****************************************************************************

vtkDataArray *
avtGenericDatabase::GetScalarVariable(const char *varname, int ts, int domain,
                                      const char *material)
{
    //
    // We have to be leery about doing any caching when the variables are
    // defined on sub-meshes.  This is because if we add new secondary
    // variables, some parts of the mesh may drop out, and the size of the
    // cached objects are now wrong.
    //
    vtkDataArray *var = NULL;
    if (!Interface->HasVarsDefinedOnSubMeshes())
    {
        var = (vtkDataArray *) cache.GetVTKObject(varname,
                         avtVariableCache::SCALARS_NAME, ts, domain, material);
    }

    if (var == NULL)
    {
        //
        // We haven't read in this domain before, so fetch it from the files.
        //
        var = Interface->GetVar(ts, domain, varname);

        if (var != NULL)
        {
            cache.CacheVTKObject(varname, avtVariableCache::SCALARS_NAME, ts,
                                 domain, material, var);

            //
            // We need to decrement the reference count of the variable 
            // returned from FetchVar, but we could not do it previously 
            // because it would knock the count down to 0 and delete it.  Since 
            // we have cached it, we can do it now.
            //
            var->Delete();
        }
    }

    return var;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetVectorVariable
//
//  Purpose:
//      Checks to see if a variable is already in cache and fetches it if it
//      is not.
//
//  Arguments:
//      varname    The name of the variable.
//      ts         The timestep for the variable.
//      domain     The domain for the variable.
//      material   The name of the material we are getting.
//
//  Returns:    A vtkDataArray for the variable.
//
//  Programmer: Hank Childs
//  Creation:   March 19, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002 
//    vtkVectors has been deprecated in VTK 4.0.  Use vtkDataArray instead.
//
//    Hank Childs, Fri Mar 14 20:56:40 PST 2003
//    Turned off caching in some instances.
//
// ****************************************************************************

vtkDataArray *
avtGenericDatabase::GetVectorVariable(const char *varname, int ts, int domain,
                                      const char *material)
{
    //
    // We have to be leery about doing any caching when the variables are
    // defined on sub-meshes.  This is because if we add new secondary
    // variables, some parts of the mesh may drop out, and the size of the
    // cached objects are now wrong.
    //
    vtkDataArray *var = NULL;
    if (!Interface->HasVarsDefinedOnSubMeshes())
    {
        var = (vtkDataArray *) cache.GetVTKObject(varname,
                         avtVariableCache::VECTORS_NAME, ts, domain, material);
    }

    if (var == NULL)
    {
        //
        // We haven't read in this domain before, so fetch it from the files.
        //
        var = Interface->GetVectorVar(ts, domain, varname);

        if (var != NULL)
        {
            cache.CacheVTKObject(varname, avtVariableCache::VECTORS_NAME, ts, 
                                 domain, material, var);

            //
            // We need to decrement the reference count of the variable 
            // returned from FetchVar, but we could not do it previously 
            // because it would knock the count down to 0 and delete it.  Since
            // we have cached it, we can do it now.
            //
            var->Delete();
        }
    }

    return var;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetTensorVariable
//
//  Purpose:
//      Checks to see if a variable is already in cache and fetches it if it
//      is not.
//
//  Arguments:
//      varname    The name of the variable.
//      ts         The timestep for the variable.
//      domain     The domain for the variable.
//      material   The name of the material we are getting.
//
//  Returns:    A vtkDataArray for the variable.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
// ****************************************************************************

vtkDataArray *
avtGenericDatabase::GetTensorVariable(const char *varname, int ts, int domain,
                                      const char *material)
{
    //
    // We have to be leery about doing any caching when the variables are
    // defined on sub-meshes.  This is because if we add new secondary
    // variables, some parts of the mesh may drop out, and the size of the
    // cached objects are now wrong.
    //
    vtkDataArray *var = NULL;
    if (!Interface->HasVarsDefinedOnSubMeshes())
    {
        var = (vtkDataArray *) cache.GetVTKObject(varname,
                         avtVariableCache::TENSORS_NAME, ts, domain, material);
    }

    if (var == NULL)
    {
        //
        // We haven't read in this domain before, so fetch it from the files.
        // Note: we use the vector var interface to get tensors.
        //
        var = Interface->GetVectorVar(ts, domain, varname);

        if (var != NULL)
        {
            cache.CacheVTKObject(varname, avtVariableCache::TENSORS_NAME, ts, 
                                 domain, material, var);

            //
            // We need to decrement the reference count of the variable 
            // returned from FetchVar, but we could not do it previously 
            // because it would knock the count down to 0 and delete it.  Since
            // we have cached it, we can do it now.
            //
            var->Delete();
        }
    }

    return var;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetSymmetricTensorVariable
//
//  Purpose:
//      Checks to see if a variable is already in cache and fetches it if it
//      is not.
//
//  Arguments:
//      varname    The name of the variable.
//      ts         The timestep for the variable.
//      domain     The domain for the variable.
//      material   The name of the material we are getting.
//
//  Returns:    A vtkDataArray for the variable.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2003
//
// ****************************************************************************

vtkDataArray *
avtGenericDatabase::GetSymmetricTensorVariable(const char *varname, int ts,
                                               int domain,const char *material)
{
    //
    // We have to be leery about doing any caching when the variables are
    // defined on sub-meshes.  This is because if we add new secondary
    // variables, some parts of the mesh may drop out, and the size of the
    // cached objects are now wrong.
    //
    vtkDataArray *var = NULL;
    if (!Interface->HasVarsDefinedOnSubMeshes())
    {
        var = (vtkDataArray *) cache.GetVTKObject(varname,
                         avtVariableCache::SYMMETRIC_TENSORS_NAME, ts, domain,
                              material);
    }

    if (var == NULL)
    {
        //
        // We haven't read in this domain before, so fetch it from the files.
        // Note: we use the vector var interface to get tensors.
        //
        var = Interface->GetVectorVar(ts, domain, varname);

        if (var != NULL)
        {
            cache.CacheVTKObject(varname, avtVariableCache::TENSORS_NAME, ts, 
                                 domain, material, var);

            //
            // We need to decrement the reference count of the variable 
            // returned from FetchVar, but we could not do it previously 
            // because it would knock the count down to 0 and delete it.  Since
            // we have cached it, we can do it now.
            //
            var->Delete();
        }
    }

    return var;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetMesh
//
//  Purpose:
//      Checks to see if a mesh is already in cache and fetches it if it's not.
//
//  Arguments:
//      meshname   The name of the mesh.
//      ts         The timestep for the mesh.
//      domain     The domain for the mesh.
//      material   The name of the material we are getting.
//
//  Returns:    A vtkDataSet for the mesh.  The caller of this routine owns
//              the mesh.
//
//  Programmer: Hank Childs
//  Creation:   October 31, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  1 09:20:52 PST 2001
//    Moved from avtDatabase to avtGenericDatabase.
//
//    Kathleen Bonnell, Tue Jun 12 14:34:02 PDT 2001 
//    Added Code to pass ghost-level information along if necessary. 
//
//    Hank Childs, Wed Aug 22 14:45:33 PDT 2001
//    Added case for poly data meshes.
//
//    Hank Childs, Mon Oct  8 10:54:30 PDT 2001
//    Added material argument.
//
//    Hank Childs, Thu Oct 11 13:04:25 PDT 2001
//    Better support for returning NULL.
//
//    Hank Childs, Wed Jan  9 14:05:05 PST 2002
//    Removed unnecessary code in favor of MakeObject.
//
//    Hank Childs, Tue Jun 25 19:44:24 PDT 2002
//    Copy over the field data.
//
//    Hank Childs, Mon Sep 30 09:11:13 PDT 2002
//    Indicate if we have ghost zones or not.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Use NewInstance instead of MakeObject, new vtk api.
//
//    Hank Childs, Fri Mar 14 20:56:40 PST 2003
//    Turned off caching in some instances.
//
//    Hank Childs, Tue Jul 29 16:48:28 PDT 2003
//    Scale mesh when appropriate.  Also cache bounds.
//
// ****************************************************************************

vtkDataSet *
avtGenericDatabase::GetMesh(const char *meshname, int ts, int domain,
                            const char *material)
{
    //
    // We have to be leery about doing any caching when the variables are
    // defined on sub-meshes.  This is because if we add new secondary
    // variables, some parts of the mesh may drop out, and the size of the
    // cached objects are now wrong.
    //
    vtkDataSet *mesh = NULL;
    if (!Interface->HasVarsDefinedOnSubMeshes())
    {
        mesh = (vtkDataSet *) cache.GetVTKObject(meshname, 
                         avtVariableCache::DATASET_NAME, ts, domain, material);
    }

    if (mesh == NULL)
    {
        //
        // We haven't read in this domain before, so fetch it from the files.
        //
        mesh = Interface->GetMesh(ts, domain, meshname);

        if (mesh == NULL)
        {
            debug5 << "Mesh returned by file format is NULL for domain "
                   << domain << ", material = " << material << endl;
            return NULL;
        }

        //
        // Force an Update.  This needs to be done and if we do it when we
        // read it in, then it guarantees it only happens once.
        //
        mesh->Update();

        AssociateBounds(mesh);
        ScaleMesh(mesh);

        cache.CacheVTKObject(meshname, avtVariableCache::DATASET_NAME, ts,
                             domain, material, mesh);

        //
        // We need to decrement the reference count of the variable returned
        // from FetchMesh, but we could not do it previously because it would
        // knock the count down to 0 and delete it.  Since we have cached it,
        // we can do it now.
        //
        mesh->Delete();
    }

    //
    // We cannot return this mesh exactly.  If we do, when we try to plot
    // another variable, it will use the same mesh and reset the variable,
    // invalidating the mesh we have in the pipeline.  To get around this, we
    // can copy the structure of the mesh (which is cheap and done through
    // reference counting).
    //
    vtkDataSet *rv = (vtkDataSet *) mesh->NewInstance();
    rv->CopyStructure(mesh);

    if (mesh->GetCellData()->GetArray("vtkGhostLevels"))
    {
        rv->GetCellData()->AddArray(
            mesh->GetCellData()->GetArray("vtkGhostLevels"));
        GetMetaData(ts)->SetContainsGhostZones(meshname, AVT_HAS_GHOSTS);
    }
    rv->GetFieldData()->ShallowCopy(mesh->GetFieldData());

    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data by asking the interface if it has it.  Also
//      performs caching.
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2001
//
//  Modifications:
//
//    Hank Childs, Tue May 22 10:47:31 PDT 2001
//    Use a database specification instead of a SIL restriction.  Also took
//    care of hacks for caching/memory management.
//
//    Hank Childs, Wed Oct 10 15:47:49 PDT 2001
//    Don't cache back NULL data.
//
// ****************************************************************************

void
avtGenericDatabase::GetAuxiliaryData(avtDataSpecification_p spec,
                                     VoidRefList &rv, const char *type, 
                                     void *args)
{
    //
    // Get the fields out of the database specification.  Use the SIL
    // specification since it captures the concept of a SIL restriction as well
    // as the concept of an object that spans all domains, like an interval
    // tree (which it represents as domain -1).
    //
    int ts = spec->GetTimestep();
    const char *var = spec->GetVariable();
    avtSILSpecification sil = spec->GetSIL();

    vector<int> domains;
    sil.GetDomainList(domains);

    //
    // Populate the void reference list.
    //
    rv.nList = domains.size();
    rv.list = new void_ref_ptr[rv.nList];

    for (int i = 0 ; i < domains.size() ; i++)
    {
        //
        // See if we already have the data lying around.
        //
        void_ref_ptr vr = cache.GetVoidRef(var, type, ts, domains[i]);
        if (*vr != NULL)
        {
            //
            // We have it, so share a reference and you're done.
            //
            rv.list[i] = vr;
        }
        else
        {
            //
            // We did not have it, so calculate it and then store it.
            //
            DestructorFunction df;
            void *d = Interface->GetAuxiliaryData(var,ts,domains[i],type,args,
                                                  df);

            if (d != NULL)
            {
                //
                // Putting 'd' into the world of reference pointers saves some
                // headaches.
                //
                void_ref_ptr vr = void_ref_ptr(d, df);
                cache.CacheVoidRef(var, type, ts, domains[i], vr);
                rv.list[i] = vr;
            }
        }
    }
}


// ****************************************************************************
//  Method: avtGenericDatabase::PopulateSIL
//
//  Purpose:
//      Populates a SIL from the database meta-data.
//
//  Arguments:
//      sil       : The sil to populate.
//      timeState : The time state that we're interested in.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
//  Modifications:
//    Hank Childs, Fri Sep  6 12:24:55 PDT 2002
//    Relocated old routine to avtSILGenerator.  Blew away outdated comments.
//
//    Brad Whitlock, Wed May 14 09:12:42 PDT 2003
//    Added optional timeState argument.
//
// ****************************************************************************

void
avtGenericDatabase::PopulateSIL(avtSIL *sil, int timeState)
{
    int timerHandle = visitTimer->StartTimer();
    avtDatabaseMetaData *md = GetMetaData(timeState);
    avtSILGenerator gen;
    gen.CreateSIL(md, sil);
    visitTimer->StopTimer(timerHandle, "Creating a SIL object.");
    visitTimer->DumpTimings();
}


// ****************************************************************************
//  Method: avtGenericDatabase::PopulateIOInformation
//
//  Purpose:
//      Populates the I/O Hints for a database, which allows domains that
//      should be processed on the same processor for I/O performance reasons
//      to do so.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2001
//
// ****************************************************************************

void
avtGenericDatabase::PopulateIOInformation(avtIOInformation &ioInfo)
{
    Interface->PopulateIOInformation(ioInfo);
}

// ****************************************************************************
//  Function: AddOriginalCellsArray
//
//  Purpose:
//    Creates an array of 'original zone numbers' to attach to the dataset.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 8, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Mon Nov 12 09:38:17 PST 2001
//    Don't re-create array if it is already present.  Made values in
//    original cell array be global in nature, so multiple-domain datasets
//    have unique "original zones" across domains.  Turn on flag that
//    ensures this array will be copied by vtk filters.
//
//    Kathleen Bonnell, Tue Mar 26 15:58:05 PST 2002 
//    Added new argument to keep track of starting cell number.
//
//    Kathleen Bonnell, Mon May 20 17:01:31 PDT 2002  
//    Changed int parameter to domain number.  Add domain as another
//    component to the array. 
//
//    Hank Childs, Sat Sep  7 08:34:08 PDT 2002
//    Sped up routine and fixed memory leak.
//
//    Hank Childs, Mon Sep 30 18:01:26 PDT 2002
//    Do not force the domain to be encoded.
//
//    Hank Childs, Fri Jan 17 08:47:57 PST 2003
//    Re-ordered the location of the start-timer call to make sure that it
//    would have a matching stop-timer.
//
// ****************************************************************************

void
avtGenericDatabase::AddOriginalCellsArray(vtkDataSet *ds, const int domain)
{
    if (ds == NULL || ds->GetCellData()->GetArray("avtOriginalCellNumbers"))
    {
        // Array already created, probably by GetMeshDataset
        return;
    }
    int timerHandle = visitTimer->StartTimer();
    vtkUnsignedIntArray *origZones = vtkUnsignedIntArray::New();
    origZones->SetName("avtOriginalCellNumbers");
    bool encodeDomains = (domain >= 0 ? true : false);
    int nComps = (encodeDomains ? 2 : 1);
    origZones->SetNumberOfComponents(nComps);
    int ncells = ds->GetNumberOfCells();
    origZones->SetNumberOfTuples(ncells);
    unsigned int *ptr = origZones->GetPointer(0);
    for (int i = 0; i < ncells; i++)
    {
        if (encodeDomains)
        {
            *ptr = domain;
            ptr++;
        }
        *ptr = i;
        ptr++;
    }
    ds->GetCellData()->AddArray(origZones);
    origZones->Delete();
    ds->GetCellData()->CopyFieldOn("avtOriginalCellNumbers");
    visitTimer->StopTimer(timerHandle, "Creating original zones array");
}


// ****************************************************************************
//  Function: AddOriginalNodesArray
//
//  Purpose:
//    Creates an array of 'original node numbers' to attach to the dataset.
//
//  Programmer: Hank Childs
//  Creation:   June 18, 2003
//
// ****************************************************************************

void
avtGenericDatabase::AddOriginalNodesArray(vtkDataSet *ds, const int domain)
{
    if (ds == NULL || ds->GetCellData()->GetArray("avtOriginalCellNumbers"))
    {
        // DataSet is NULL or array is already created -- return.
        return;
    }

    int timerHandle = visitTimer->StartTimer();
    vtkUnsignedIntArray *origNodes = vtkUnsignedIntArray::New();
    origNodes->SetName("avtOriginalNodeNumbers");
    bool encodeDomains = (domain >= 0 ? true : false);
    int nComps = (encodeDomains ? 2 : 1);
    origNodes->SetNumberOfComponents(nComps);
    int nnodes = ds->GetNumberOfPoints();
    origNodes->SetNumberOfTuples(nnodes);
    unsigned int *ptr = origNodes->GetPointer(0);
    for (int i = 0; i < nnodes; i++)
    {
        if (encodeDomains)
        {
            *ptr = domain;
            ptr++;
        }
        *ptr = i;
        ptr++;
    }
    ds->GetPointData()->AddArray(origNodes);
    origNodes->Delete();
    ds->GetPointData()->CopyFieldOn("avtOriginalNodeNumbers");
    visitTimer->StopTimer(timerHandle, "Creating original nodes array");
}


// ****************************************************************************
//  Method: avtGenericDatabase::MaterialSelect
//
//  Purpose:
//      Determines if the dataset needs to be material selected and performs
//      the selection if it does.
//
//  Arguments:
//      ds      The input as a vtk dataset.
//      mat     The avt material.
//      mvl     A list of the mixed variables.
//      dom     The domain number for this dataset.
//      var     The variable for this dataset (to determine the material).
//      ts      The timestep of interest.
//      mnames  The names of the materials we want.
//      reUseMIR  Whether or not the MIR can be re-used.
//
//  Output Arguments:
//      subdivisionOccurred              true if subdiv occurred in MIR
//      notAllCellsSubdivided            true if some cells were left whole
//
//  Returns:    A data tree representation of the dataset, material selected or
//              not based on the sil restriction.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2001
//
//  Arguments:
//
//    Hank Childs, Tue Jul 24 14:17:14 PDT 2001
//    Added didIt argument.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001 
//    Retrieve material names and pass them to the output tree. 
//
//    Jeremy Meredith, Mon Oct  1 12:09:06 PDT 2001
//    Pass mixvar info to GetZonalVariable.
//
//    Hank Childs, Fri Oct  5 16:27:34 PDT 2001
//    Added mnames argument and removed arguments didIt and silr.
//
//    Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001 
//    Added labels argument, to be used when creating the data tree instead
//    of mnames. 
//
//    Hank Childs, Mon Nov 26 09:01:36 PST 2001
//    Added material argument.
//
//    Hank Childs, Wed Dec  5 10:57:23 PST 2001
//    Added mixvar argument.
//
//    Eric Brugger, Tue Dec 11 13:02:39 PST 2001
//    I modified the material selection to create a single dataset with
//    all the cells instead of a dataset per material.  This causes the
//    internal material boundaries to be eliminated.
//
//    Jeremy Meredith, Wed Dec 19 20:13:05 PST 2001
//    Added understanding of species as a zonal variable.
//
//    Hank Childs, Fri Feb  1 15:20:32 PST 2002
//    Material select all of the variables.
//
//    Hank Childs, Wed Feb  6 11:45:12 PST 2002
//    Check before overwriting ghost levels array.
//
//    Jeremy Meredith, Thu Mar 14 18:02:31 PST 2002
//    Added a flag to split materials into their own datasets.
//
//    Jeremy Meredith, Thu Mar 14 18:03:22 PST 2002
//    Added a flag for the helper MaterialSelect which says whether or
//    not the dataspec says we need internal surfaces.
//
//    Kathleen Bonnell, Fri Mar 15 10:11:23 PST 2002   
//    Use new VTK methods for changing active scalars in PointData and CellData.
//
//    Hank Childs, Thu Jul  4 09:30:05 PDT 2002
//    Fit MIRs new interface to handle secondary variables without so much
//    hoopla.  Also add support for an arbitrary number of mixvars.
//
//    Jeremy Meredith, Tue Aug 13 14:36:40 PDT 2002
//    Added the needValidConnectivity flag, as well as the two subdivision
//    return flags.
//
//    Kathleen Bonnell, Thu Aug 15 20:08:34 PDT 2002  
//    Use the blockOrigin when setting a domain label (instead of adding 1
//    to the domain number). 
//    
//    Hank Childs, Sun Aug 18 22:33:35 PDT 2002
//    Added argument for whether or not the MIR can be re-used.
//
//    Kathleen Bonnell, Thu Sep  5 13:53:15 PDT 2002  
//    Added support for creating group labels. 
//
//    Hank Childs, Thu Sep 26 08:22:40 PDT 2002
//    Cleaned up huge memory leak.
//
//    Hank Childs, Wed Oct  2 10:09:53 PDT 2002
//    Go back to using reference pointers because the MIR may not have been
//    cached.
//
//    Jeremy Meredith, Thu Oct 24 15:37:05 PDT 2002
//    Added smoothing option and clean zones only option.
//
//    Jeremy Meredith, Thu Jun 12 09:07:28 PDT 2003
//    Added needBoundarySurfaces option.
//
//    Hank Childs, Tue Jul 22 21:48:09 PDT 2003
//    Added a flag for whether or not we communicated ghosts.
//
//    Hank Childs, Fri Sep 12 16:27:38 PDT 2003
//    Added a flag for whether or not reconstruction was forced.
//
//    Jeremy Meredith, Fri Sep  5 15:31:53 PDT 2003
//    Added a flag for the MIR algorithm.
//
//    Kathleen Bonnell, Thu Sep 18 11:49:16 PDT 2003 
//    Add 'vtkOriginalDimensions' field data to output, if input was
//    structured (so Pick can return correct zone/node numbers).
//
//    Jeremy Meredith, Wed Oct 15 17:25:48 PDT 2003
//    Added code to correctly handle clean-zones-only MIR.
//
// ****************************************************************************

avtDataTree_p
avtGenericDatabase::MaterialSelect(vtkDataSet *ds, avtMaterial *mat,
                        vector<avtMixedVariable *> mvl,int dom,const char *var,
                        int ts, vector<string> &mnames, vector<string> &labels,
                        bool needInternalSurfaces,
                        bool needBoundarySurfaces,
                        bool needValidConnectivity,
                        bool needSmoothMaterialInterfaces,
                        bool needCleanZonesOnly,
                        bool reconstructionForced,
                        int  mirAlgorithm,
                        bool didGhosts,
                        bool &subdivisionOccurred,
                        bool &notAllCellsSubdivided,
                        bool reUseMIR)
{
    //
    // We need to have the material indices as well.
    //
    vector<int> mindex;
    GetMaterialIndices(var, mnames, mindex);

    //
    // Make room for the "mixed" material (i.e. for clean-zones-only)
    //
    mindex.push_back(mat->GetNMaterials());

    //
    // Determine the topological dimension.
    //
    avtDatabaseMetaData *md = GetMetaData(ts);
    string meshname = md->MeshForVar(var);
    const avtMeshMetaData *mmd = md->GetMesh(meshname);
    if (mmd == NULL)
    {
        return NULL;
    }
    int topoDim = mmd->topologicalDimension;

    void_ref_ptr vr_mir = GetMIR(dom, var, ts, ds, mat, topoDim,
                                 needValidConnectivity,
                                 needSmoothMaterialInterfaces,
                                 needCleanZonesOnly, mirAlgorithm,
                                 didGhosts,
                                 subdivisionOccurred,
                                 notAllCellsSubdivided, reUseMIR);
    MIR *mir = (MIR *) (*vr_mir);

    int numSelected = mindex.size();

    vector<int> selMats;
    int numOutput = (needInternalSurfaces ?  numSelected : 1);
    vtkDataSet **out_ds = new vtkDataSet *[numOutput];

    avtVarType type = GetMetaData(ts)->DetermineVarType(var);
    vtkIntArray *origDims = NULL;
    if (ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        int *dims = ((vtkStructuredGrid*)ds)->GetDimensions();
        origDims = vtkIntArray::New();
        origDims->SetName("vtkOriginalDimensions");
        origDims->SetNumberOfComponents(1);
        origDims->SetNumberOfTuples(3);
        origDims->SetValue(0, dims[0]);
        origDims->SetValue(1, dims[1]);
        origDims->SetValue(2, dims[2]);
    }
    else if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        int *dims = ((vtkRectilinearGrid*)ds)->GetDimensions();
        origDims = vtkIntArray::New();
        origDims->SetName("vtkOriginalDimensions");
        origDims->SetNumberOfComponents(1);
        origDims->SetNumberOfTuples(3);
        origDims->SetValue(0, dims[0]);
        origDims->SetValue(1, dims[1]);
        origDims->SetValue(2, dims[2]);
    }
   

    for (int d=0; d<numOutput; d++)
    {
        if (needInternalSurfaces)
        {
            selMats.clear();
            selMats.push_back(mindex[d]);
        }
        else
        {
            selMats = mindex;
        }

        bool  setUpMaterialVariable = false;
        if (type == AVT_MATERIAL || reconstructionForced)
        {
            setUpMaterialVariable = true;
        }

        out_ds[d] = mir->GetDataset(selMats, ds, mvl, setUpMaterialVariable,
                                    mat);

        if (out_ds != NULL && out_ds[d]->GetNumberOfCells() == 0)
        {
            out_ds[d]->Delete();
            out_ds[d] = NULL;
        }

        if (out_ds[d] && needBoundarySurfaces)
        {
            //
            // We need to extract the internal boundaries
            //
            if (out_ds[d]->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
            {
                EXCEPTION1(ImproperUseException, "MaterialSelect did not "
                           "get a VTK_UNSTRUCTURED_GRID\n")
            }

            vtkUnstructuredGridBoundaryFilter *bf =
                vtkUnstructuredGridBoundaryFilter::New();

            vtkDataSet *in_ds = out_ds[d];

            bf->SetInput((vtkUnstructuredGrid*)in_ds);
            out_ds[d] = bf->GetOutput();
            bf->Update();

            out_ds[d]->Register(NULL);
            out_ds[d]->SetSource(NULL);
            bf->Delete();

            if (out_ds != NULL && out_ds[d]->GetNumberOfCells() == 0)
            {
                out_ds[d]->Delete();
                out_ds[d] = NULL;
            }

        }
        if (out_ds[d] && origDims)
        {
            out_ds[d]->GetFieldData()->AddArray(origDims);
            out_ds[d]->GetFieldData()->CopyFieldOn("vtkOriginalDimensions");
        }
    }
    if (origDims)
        origDims->Delete();

    //
    // Create new labels.
    //
    vector<string> labelStrings;

    if (type == AVT_MATERIAL)
    {
        if (needInternalSurfaces)
        {
            labelStrings = labels;
            // add the one for the "mixed material"
            labelStrings.push_back("mixed");
        }
        else
        {
            //
            // Create a new label of the form "%d;%d;%s;...", which consists
            // of the number of materials, followed by pairs of material
            // number and material name for each material.  Make sure the last
            // one is for the "mixed material".
            //
            char   buff[32];
            string label;
            sprintf(buff, "%d;", numSelected);
            label += buff;
            for (int i = 0; i < numSelected; i++)
            {
                sprintf(buff, "%d;", mindex[i]);
                if (i == numSelected-1)
                    label += string(buff) + "mixed;";
                else
                    label += string(buff) + labels[i] + ";";
            }
            labelStrings.push_back(label);
        }
    }
    else
    {
        char label[40];
        avtSubsetType sT = GetMetaData(ts)->DetermineSubsetType(var);
        if (sT == AVT_DOMAIN_SUBSET || sT == AVT_UNKNOWN_SUBSET)
        {
            //
            // Create a new label which is the domain number.
            // We are doing a material-selected domain-subset plot,
            // or a material-selected non-subset plot. 
            //
            int domOrigin  =  GetMetaData(ts)->GetMesh(meshname)->blockOrigin;
            sprintf(label, "%d", dom + domOrigin);
            for (int d = 0; d < numOutput; d++)
            {
                labelStrings.push_back(label);
            }
        }
        else if (sT == AVT_GROUP_SUBSET)
        {
            //
            // Create a new label which is the group number.
            //
            int gID  =  GetMetaData(ts)->GetMesh(meshname)->groupIds[dom];
            sprintf(label, "%d", gID);
            for (int d = 0; d < numOutput; d++)
            {
                labelStrings.push_back(label);
            }
        }
    }

    avtDataTree_p outDT = new avtDataTree(numOutput, out_ds, dom, labelStrings);
    for (int i = 0 ; i < numOutput ; i++)
    {
        if (out_ds[i] != NULL)
        {
            out_ds[i]->Delete();
        }
    }
    delete [] out_ds;

    return outDT;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetMaterialIndices
//
//  Purpose:
//      Gets a list of material indices based on a list of material names.
//
//  Arguments:
//      var     The variable we want.
//      mn      The material names.
//      ml      A place to put the material list.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2001
//
// ****************************************************************************

void
avtGenericDatabase::GetMaterialIndices(const char *var, vector<string> &mn,
                                       vector<int> &ml)
{
    // assume material composition is invariant with time, get at time 0
    avtDatabaseMetaData *md = GetMetaData(0);
    string meshname = md->MeshForVar(var);
    string matname  = md->MaterialOnMesh(meshname);
    const avtMaterialMetaData *mmd = md->GetMaterial(matname);
    if (mmd == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }
    int nstr = mn.size();
    for (int i = 0 ; i < nstr ; i++)
    {
        bool foundMatch = false;
        for (int j = 0 ; j < mmd->numMaterials ; j++)
        {
            if (mn[i] == mmd->materialNames[j])
            {
                ml.push_back(j);
                foundMatch = true;
                break;
            }
        }
        if (!foundMatch)
        {
            EXCEPTION0(ImproperUseException);
        }
    }
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetMaterial
//
//  Purpose:
//      Gets a material for a specific domain and a variable.  This handles
//      going through the auxiliary data mechanism and caching issues.
//
//  Arguments:
//      dom     The domain we want the material for.
//      var     A variable (could be vector, scalar, mesh, or material).
//      ts      The timestep of interest.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 4 16:25:10 PST 2002
//    Changed CopyTo to an inline template function.
//
// ****************************************************************************

avtMaterial *
avtGenericDatabase::GetMaterial(int dom, const char *var, int ts)
{
    avtDatabaseMetaData *md = GetMetaData(ts);

    //
    // Identify the material we want.
    //
    string meshname = md->MeshForVar(var);
    string matname  = md->MaterialOnMesh(meshname);

    avtDataSpecification_p spec = new avtDataSpecification(matname.c_str(),
                                                           ts, dom);
    VoidRefList mats;
    GetAuxiliaryData(spec, mats, AUXILIARY_DATA_MATERIAL, NULL);
    if (mats.nList != 1)
    {
        EXCEPTION0(ImproperUseException);
    }
    
    avtMaterial *rv = (avtMaterial *) *(mats.list[0]);

    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetSpecies
//
//  Purpose:
//      Gets a species for a specific domain and a variable.  This handles
//      going through the auxiliary data mechanism and caching issues.
//
//  Arguments:
//      dom     The domain we want the species for.
//      var     A variable (could be vector, scalar, mesh, mat, or species).
//      ts      The timestep of interest.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 17, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Apr 4 16:25:10 PST 2002
//    Changed CopyTo to an inline template function.
//
// ****************************************************************************

avtSpecies *
avtGenericDatabase::GetSpecies(int dom, const char *var, int ts)
{
    avtDatabaseMetaData *md = GetMetaData(ts);

    //
    // Identify the species we want.
    //
    string meshname = md->MeshForVar(var);
    string specname = md->SpeciesOnMesh(meshname);

    avtDataSpecification_p dspec = new avtDataSpecification(specname.c_str(),
                                                            ts, dom);
    VoidRefList specs;
    GetAuxiliaryData(dspec, specs, AUXILIARY_DATA_SPECIES, NULL);
    if (specs.nList != 1)
    {
        EXCEPTION0(ImproperUseException);
    }
    
    avtSpecies *rv = (avtSpecies *) *(specs.list[0]);

    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::SpeciesSelect
//
//  Purpose:
//      Performs species selection.
//
//  Arguments:
//      dsc       The dataset collection.  This will be modified.
//      domains   The domain list.
//      specList  A list of species that are on and off.
//      spec      The data specification.
//      src       The source from a database.  Used for progress.
//
//  Programmer:   Hank Childs
//  Creation:     December 19, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  8 08:48:06 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Thu Jul  4 14:53:05 PDT 2002
//    Reflect interface change that supports multiple mixed variables.
//
//    Hank Childs, Tue Sep 17 10:19:15 PDT 2002
//    Add support for all secondary variables.  Also fix memory leak.
//
//    Hank Childs, Tue Sep 24 08:25:28 PDT 2002
//    I screwed up the progress bar for species selection on my last checkin.
//
// ****************************************************************************

void
avtGenericDatabase::SpeciesSelect(avtDatasetCollection &dsc, 
                      vector<int> &domains, vector<bool> &specList,
                      avtDataSpecification_p &spec, avtSourceFromDatabase *src)
{
    char *progressString = "Doing species selection";
    src->DatabaseProgress(0, 0, progressString);

    int timestep = spec->GetTimestep();

    int nDomains = dsc.GetNDomains();
    for (int i = 0 ; i < nDomains ; i++)
    {
        int m = 0; // Using 0 assumes that the file format does not do matsel. 
        vtkDataSet *ds = dsc.GetDataset(i, m);

        int n_cell_vars = ds->GetCellData()->GetNumberOfArrays();
        // If we don't make a copy of the cell data and then go mucking with
        // the order of the arrays, we may process some twice and not others.
        vtkCellData *inCD = vtkCellData::New();
        inCD->ShallowCopy(ds->GetCellData());
        for (int j = 0 ; j < n_cell_vars ; j++)
        {
            vtkDataArray *arr = inCD->GetArray(j);
            if (arr->GetDataType() != VTK_FLOAT ||
                arr->GetNumberOfComponents() != 1)
            {
                continue;
            }

            string var = arr->GetName();
            bool activeVar = false;
            if (ds->GetCellData()->GetScalars() == arr)
            {
                activeVar = true;
            }
            avtMaterial *mat = GetMaterial(domains[i], var.c_str(), timestep);
            avtSpecies *species = GetSpecies(domains[i],var.c_str(),timestep);

            avtMixedVariable *mixvar = (avtMixedVariable *)
                                                       *(dsc.GetMixVar(i,var));

            avtMixedVariable *mixVarOut = NULL;
            vtkDataArray     *scalarOut = NULL;
            MIR::SpeciesSelect(specList, mat, species, arr, mixvar, 
                               scalarOut, mixVarOut);
            ds->GetCellData()->RemoveArray(var.c_str());
            ds->GetCellData()->AddArray(scalarOut);
            if (activeVar)
            {
                ds->GetCellData()->SetActiveScalars(var.c_str());
            }
            scalarOut->Delete();
            if (mixVarOut != NULL)
            {
                void_ref_ptr vr = void_ref_ptr(mixVarOut,
                                               avtMixedVariable::Destruct);
                dsc.AddMixVar(i, vr);
            }
            src->DatabaseProgress(i*n_cell_vars+j, nDomains*n_cell_vars,
                                  progressString);
        }
        inCD->Delete();
    }

    src->DatabaseProgress(1, 0, progressString);
}


// ****************************************************************************
//  Method: avtGenericDatabase::GetMIR
//
//  Purpose:
//      Gets the Material Interface Reconstruction (MIR), by first checking
//      to see if it was cached and returning the cached object if it was.
//      It then goes ahead and retrieves the material and reconstructs the
//      mesh if necessary.
//
//  Arguments:
//      domain    The domain of interest.
//      varname   The name of the variable (not necessarily the material name).
//      timestep  The timestep of interest.
//      ds        The dataset to reconstruct.
//      mat       The material.
//      topoDim   Whether this is a 3D mesh or 2D mesh.
//      needValidConnectivity
//                True if any subdivision must imply complete subdivision
//      reUseMIR  True if we can use this MIR on subsequent runs.
//
//  Output Arguments:
//      subdivisionOccurred              true if subdiv occurred in MIR
//      notAllCellsSubdivided            true if some cells were left whole
//
//  Returns:      The MIR object.
//
//  Programmer:   Hank Childs
//  Creation:     July 23, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Nov 26 09:06:07 PST 2001
//    Added material argument.  This allowed for the material to be read in
//    at the same time as the mesh, which allowed for ghost zone information
//    to be communicated.
//
//    Brad Whitlock, Thu Apr 4 16:25:10 PST 2002
//    Changed CopyTo to an inline template function.
//
//    Jeremy Meredith, Tue Aug 13 14:34:20 PDT 2002
//    Allowed leaving clean zones whole when possible.  This means
//    two *different* MIRs need to be in the cache.
//
//    Hank Childs, Sun Aug 18 22:33:35 PDT 2002
//    Add a boolean argument about whether we can re-use the MIR.
//
//    Hank Childs, Tue Sep 24 15:24:56 PDT 2002
//    Remove unneeded reference counting.
//
//    Hank Childs, Wed Oct  2 08:15:53 PDT 2002
//    Make the return type be a void_ref_ptr.
//
//    Jeremy Meredith, Thu Oct 24 15:36:34 PDT 2002
//    Added smoothing option and clean zones only option.
//
//    Hank Childs, Tue Jul 22 21:48:09 PDT 2003
//    Added a flag for whether or not we communicated ghosts.
//
//    Jeremy Meredith, Fri Sep  5 15:31:33 PDT 2003
//    Added the new MIR algorithm.
//
//    Jeremy Meredith, Thu Sep 18 11:31:23 PDT 2003
//    Made the new MIR algorithm work in 2D as well.
//
// ****************************************************************************
void_ref_ptr
avtGenericDatabase::GetMIR(int domain, const char *varname, int timestep,
                           vtkDataSet *ds, avtMaterial *mat, int topoDim,
                           bool needValidConnectivity,
                           bool needSmoothMaterialInterfaces,
                           bool needCleanZonesOnly, 
                           int  mirAlgorithm,
                           bool didGhosts,
                           bool &subdivisionOccurred,
                           bool &notAllCellsSubdivided, bool reUseMIR)
{
    char cacheLbl[1000];
    sprintf(cacheLbl, "MIR_%s_%s_%s_%s_%s",
            needValidConnectivity        ? "FullSubdiv" : "MinimalSubdiv",
            needSmoothMaterialInterfaces ? "Smooth"     : "NotSmooth",
            needCleanZonesOnly           ? "CleanOnly"  : "SplitMixed",
            didGhosts                    ? "DidGhosts"  : "NoDidGhosts",
            mirAlgorithm==0              ? "TetMIR"     : "ZooMIR");

    //
    // See if we already have the data lying around.
    //
    avtDatabaseMetaData *md = GetMetaData(timestep);
    string meshname = md->MeshForVar(varname);
    string matname  = md->MaterialOnMesh(meshname);
    void_ref_ptr vr = void_ref_ptr();
    if (reUseMIR)
    {
        vr = cache.GetVoidRef(matname.c_str(), cacheLbl, timestep, domain);
    }

    if (*vr == NULL)
    {
        //
        // We will need the material to calculate the reconstruction.
        //
        if (mat == NULL)
        {
            EXCEPTION0(NoInputException);
        }

        MIR *mir = NULL;

        //
        // Right new the new algorithm (index==1) is only
        // available in 3D.
        //
        if (mirAlgorithm == 1)
            mir = new ZooMIR;
        else
            mir = new TetMIR;

        mir->SetLeaveCleanZonesWhole(!needValidConnectivity);
        mir->SetSmoothing(needSmoothMaterialInterfaces);
        mir->SetCleanZonesOnly(needCleanZonesOnly);
        if (topoDim == 3)
        {
            mir->Reconstruct3DMesh(ds, mat);
        }
        else
        {
            mir->Reconstruct2DMesh(ds, mat);
        }

        //
        // Putting the MIR into the world of reference pointers saves some
        // headaches.
        //
        vr = void_ref_ptr(mir, MIR::Destruct);
        
        if (reUseMIR)
        {
            cache.CacheVoidRef(matname.c_str(), cacheLbl, timestep, domain,vr);
        }
    }

    MIR *rv = (MIR *) *vr;
    subdivisionOccurred   = rv->SubdivisionOccurred();
    notAllCellsSubdivided = rv->NotAllCellsSubdivided();
    return vr;
}


// ****************************************************************************
//  Method: avtGenericDatabase::PrepareMaterialSelect
//
//  Purpose:
//      Determines if we need to do a material selection.  Notifies the file
//      format interface of the materials if we should do a material selection,
//      so that it can do the material selection as we read.
//
//  Arguments:
//      dom          The domain.
//      forceMIROn   A boolean saying whether we should force MIR on.
//      silr         The SIL restriction.
//      mnames       A place to put the list of material names.
//      
//  Returns:    true if we need to perform material selection, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov 22 16:39:13 PST 2002
//    Use the SIL restriction traverser since SIL restriction routines were
//    antiquated.
//
//    Hank Childs, Wed Aug 13 08:09:20 PDT 2003
//    Do not longer key off type, instead use a flag that has been explicitly
//    set.
//
// ****************************************************************************

bool
avtGenericDatabase::PrepareMaterialSelect(int dom, bool forceMIROn,
                      avtSILRestrictionTraverser &trav, vector<string> &mnames)
{
    //
    // Have the SIL restriction determine if material selection is necessary
    // for this domain.
    //
    bool needMatSel;
    mnames = trav.GetMaterials(dom, needMatSel);

    //
    // If we were told to do material interface reconstruction, we should do
    // it, regardless of SIL.
    //
    needMatSel |= forceMIROn;

    return needMatSel;
}


// ****************************************************************************
//  Method: avtGenericDatabase::CanDoDynamicLoadBalancing
//
//  Purpose:
//      Determines whether or not we can do dynamic load balancing.
//
//  Programmer: Hank Childs
//  Creation:   October 25, 2001
//
// ****************************************************************************

bool
avtGenericDatabase::CanDoDynamicLoadBalancing(void)
{
    return Interface->CanDoDynamicLoadBalancing();
}

// ****************************************************************************
//  Method: avtGenericDatabase::HasInvariantMetaData
//
//  Purpose:
//      Indicates whether or not metadata can vary with time 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 30, 2003 
//
// ****************************************************************************

bool
avtGenericDatabase::HasInvariantMetaData(void) const
{
    return Interface->HasInvariantMetaData();
}

// ****************************************************************************
//  Method: avtGenericDatabase::HasInvariantSIL
//
//  Purpose:
//      Indicates whether or not the SIL can vary with time 
//
//  Programmer: Mark C. Miller 
//  Creation:   September 30, 2003 
//
// ****************************************************************************

bool
avtGenericDatabase::HasInvariantSIL(void) const
{
    return Interface->HasInvariantSIL();
}


// ****************************************************************************
//  Method: avtGenericDatabase::ReadDataset
//
//  Purpose:
//      Reads in a dataset through the file format interface.
//
//  Arguments:
//      ds        The dataset collection.
//      domains   A list of domains to be read.
//      spec      A data specification.
//      src       The source object.
//
//  Programmer:   Hank Childs
//  Creation:     October 26, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 28 13:01:00 PST 2001
//    Remove ghost zone arrays for filters where ghost zones are not
//    appropriate.
//
//    Hank Childs, Wed Dec  5 10:33:52 PST 2001
//    Read in mixed variables so that they can be communicated in the ghost
//    zone communication phase (if appropriate).
//
//    Hank Childs, Thu Jul  4 14:53:05 PDT 2002
//    Reflect interface change that supports multiple mixed variables.
//
//    Kathleen Bonnell, Wed Sep  4 14:57:31 PDT 2002 
//    Replace NeedDomainLabels with a check of subset type to determine
//    what kind of labels get created, if any. 
//
//    Jeremy Meredith, Thu Oct 24 16:04:22 PDT 2002
//    Reordered a big chunk of code to allow users to force material
//    interface reconstruction if they have mixed variables.
//
//    Hank Childs, Tue Oct 29 07:05:36 PST 2002
//    Changed the location of the declaration of the 'labels' string to inside
//    a for loop, ensuring that it will be properly cleaned out over each
//    iteration of the loop.
//
//    Hank Childs, Fri Nov 22 16:39:13 PST 2002
//    Use the SIL restriction traverser since SIL restriction routines were
//    antiquated.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003 
//    Initialize ContainsOriginalCells in the MetaData. 
//    
// ****************************************************************************

void
avtGenericDatabase::ReadDataset(avtDatasetCollection &ds, vector<int> &domains,
                      avtDataSpecification_p &spec, avtSourceFromDatabase *src)
{
    int timerHandle = visitTimer->StartTimer();
    int ts = spec->GetTimestep();

    //
    // This will get unset later if we determine that we actually do have
    // ghosts.
    //
    avtDatabaseMetaData *md = GetMetaData(ts);
    string meshname = md->MeshForVar(spec->GetVariable());
    md->SetContainsGhostZones(meshname, AVT_NO_GHOSTS);
    md->SetContainsOriginalCells(meshname, false);

    //
    // Set up some things we will want for later.
    //
    const char *var = spec->GetVariable();
    const vector<CharStrRef> &vars2nd = spec->GetSecondaryVariables();
    avtSILRestriction_p silr = spec->GetRestriction();

    char  progressString[1024];
    sprintf(progressString, "Reading from %s", Interface->GetType());

    vector<string> blockNames;
    vector<int>    gIds;
    int domOrigin = 0;

    avtSubsetType subT = GetMetaData(ts)->DetermineSubsetType(var);
    if (subT == AVT_DOMAIN_SUBSET || subT == AVT_GROUP_SUBSET)
    {
        string meshName = GetMetaData(ts)->MeshForVar(var);
        blockNames =  GetMetaData(ts)->GetMesh(meshName)->blockNames;
        domOrigin  =  GetMetaData(ts)->GetMesh(meshName)->blockOrigin;
        gIds       =  GetMetaData(ts)->GetMesh(meshName)->groupIds;
    }

    //
    // Some file formats have variables that are defined for only some of
    // the materials.  In that case, we have to tell the file format interface
    // of all of the variables we will be interested in.
    //
    Interface->RegisterVariableList(var, vars2nd);

    //
    // Iterate through each of the domains and do material selection as we go.
    //
    src->DatabaseProgress(0, 0, progressString);
    int nDomains = domains.size();
    avtSILRestrictionTraverser trav(silr);
    for (int i = 0 ; i < nDomains ; i++)
    {
        vector<string> labels;
        vector<string> matnames;
        bool forceMIR = spec->MustDoMaterialInterfaceReconstruction();
        bool doSelect = PrepareMaterialSelect(domains[i], forceMIR, trav, 
                                              matnames);
        int nmats = matnames.size();
        vtkDataSet *single_ds = NULL;

        if (!doSelect || !Interface->PerformsMaterialSelection())
        {
            // We know we want the dataset as a whole
            debug5 << "Generic database instructing get for var = " 
                   << var << ", timestep = " << ts << " domain = "
                   << domains[i] << endl;
            single_ds = GetDataset(var, ts, domains[i], "_all", vars2nd, src);

            // Determine if there are mixed vars.  If so, force
            // material selection if it was requested for mixed vars
            if (spec->NeedMixedVariableReconstruction())
            {
                bool hasmixvar = cache.HasVoidRef(var,
                                                  AUXILIARY_DATA_MIXED_VARIABLE,
                                                  ts, domains[i]);
                for (int kk = 0 ; kk < vars2nd.size() && !hasmixvar ; kk++)
                {
                    hasmixvar |= cache.HasVoidRef(*(vars2nd[kk]),
                                                  AUXILIARY_DATA_MIXED_VARIABLE,
                                                  ts, domains[i]);
                }

                if (hasmixvar)
                    doSelect = true;
            }

            // if we're not doing material selection for sure, set nmats to 1
            if (!doSelect)
                nmats = 1;
        }

        //
        //  Allocate space for our dataset
        //
        ds.SetNumMaterials(i, nmats);

        //
        //  Prepare labels; we need nmats labels.
        //
        if (subT == AVT_DOMAIN_SUBSET)
        { 
            if (blockNames.empty())
            {
                char temp[512];
                sprintf(temp, "%d", domains[i] + domOrigin);
                for (int l = 0; l < nmats; l++)
                {
                    labels.push_back(temp);
                }
            }
            else 
            {
                for (int l = 0; l < nmats; l++)
                    labels.push_back(blockNames[domains[i]]);
            }
        }
        else if (subT == AVT_GROUP_SUBSET)
        {
            char temp[512];
            sprintf(temp, "%d", gIds[domains[i]]);
            for (int l = 0; l < nmats; l++)
                labels.push_back(temp);
        }
        else if (subT == AVT_MATERIAL_SUBSET)
        {
            labels = matnames;
        }

        ds.labels[i] = labels;

        //
        //  Finish setting up the dataset
        //
        if (doSelect)
        {
            if (Interface->PerformsMaterialSelection())
            {
                ds.needsMatSelect[i] = false;

                //
                // This file format does its own material selection, so get the
                // dataset one material at a time.
                //
                for (int j = 0 ; j < nmats ; j++)
                {
                    debug5 << "Generic database instructing get for var = " 
                           << var << ", timestep = " << ts << " domain = "
                           << domains[i] << ", material = " 
                           << matnames[j].c_str() << endl;
                    vtkDataSet *ds1 = GetDataset(var, ts, domains[i],
                                          matnames[j].c_str(), vars2nd, src);
                    ds.SetDataset(i, j, ds1);
                    if (ds1 != NULL)
                    {
                        ds1->Delete();
                    }
                }
            }
            else
            {
                ds.needsMatSelect[i] = true;

                ds.SetDataset(i, 0, single_ds);
                if (single_ds != NULL)
                {
                    single_ds->Delete();
                }

                // Get the material
                ds.matnames[i] = matnames;
                avtMaterial *mat = GetMaterial(domains[i], var, ts);
                ds.SetMaterial(i, mat);

                // Get the mixed variables
                void_ref_ptr vr=cache.GetVoidRef(var,
                                                 AUXILIARY_DATA_MIXED_VARIABLE,
                                                 ts, domains[i]);
                if (*vr != NULL)
                {
                    ds.AddMixVar(i, vr);
                }
                for (int kk = 0 ; kk < vars2nd.size() ; kk++)
                {
                    vr = cache.GetVoidRef(*(vars2nd[kk]),
                                          AUXILIARY_DATA_MIXED_VARIABLE, ts,
                                          domains[i]);
                    if (*vr != NULL)
                    {
                         ds.AddMixVar(i, vr);
                    }
                }
            }

            src->InvalidateZones();
        }
        else
        {
            ds.needsMatSelect[i] = false;

            ds.SetDataset(i, 0, single_ds);
            if (single_ds != NULL)
            {
                single_ds->Delete();
            }
        }

        src->DatabaseProgress(i, nDomains, progressString);
    }

    //
    // Sometimes we can't stop our readers from reading in ghost zones.
    // Remove the array in that case so we don't get confused downstream.
    //
    if (!spec->GhostZonesAreAppropriate())
    {
        for (int j = 0 ; j < nDomains ; j++)
        {
            vtkDataSet *ds1 = ds.GetDataset(j, 0);
            if (ds1 != NULL)
            {
                ds1->GetCellData()->RemoveArray("vtkGhostLevels");
            }
        }
    }

    src->DatabaseProgress(1, 0, progressString);

    visitTimer->StopTimer(timerHandle, "Reading dataset");
    visitTimer->DumpTimings();
}


// ****************************************************************************
//  Method: avtGenericDatabase::CommunicateGhosts
//
//  Purpose:
//      Uses a domain boundary information object to communicate the ghost
//      zones.
//
//  Arguments:
//      ds        The dataset collection.
//      doms      A list of domains.
//      spec      A data specification.
//      src       The source object.
//
//  Returns:      True if ghost zones were communicated.  False otherwise.
//
//  Programmer:   Hank Childs
//  Creation:     October 26, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Tue Dec  4 13:52:39 PST 2001
//    Added material and mixvar support.  Added parallel and progress support.
//
//    Jeremy Meredith, Fri Dec 21 12:25:29 PST 2001
//    Added species-scalar support.
//
//    Hank Childs, Wed Jan  2 08:35:34 PST 2002
//    Communicate secondary variables.
//
//    Hank Childs, Sun Jan 13 14:04:32 PST 2002
//    Clean up hack that resulted from poorly understood circumstances.
//
//    Hank Childs, Wed Mar 27 08:38:02 PST 2002
//    Make sure that the mesh we want to exchange ghost zones for is the mesh
//    that we have boundary information for.
//
//    Kathleen Bonnell, Wed Mar 27 15:47:14 PST 2002   
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Fri May 17 12:52:08 PDT 2002  
//    Make sure avtOriginalCellNumbers is exhanged if needed. 
//    
//    Hank Childs, Tue Jun 25 20:34:15 PDT 2002
//    Copy over the field data.
//
//    Hank Childs, Thu Jul  4 14:53:05 PDT 2002
//    Reflect interface change that supports multiple mixed variables.
//
//    Hank Childs, Wed Jul 10 11:33:20 PDT 2002
//    A reference was being removed from a scalar variable that should not
//    have been.
//
//    Kathleen Bonnell, Wed Jul 10 16:02:56 PDT 2002 
//    Copy FieldData arrays individually, as ExchangeMesh may set
//    an array in the NewList, and ShallowCopy will overwrite that.  
//   
//    Hank Childs, Sun Aug 18 22:33:35 PDT 2002
//    Added return value.
//
//    Hank Childs, Mon Sep 30 09:29:36 PDT 2002
//    Tell output that we have ghost zones.
//
//    Hank Childs, Wed Oct  2 10:09:53 PDT 2002
//    Also communicate the original zone numbers if we are creating structured
//    indices.
//
//    Hank Childs, Wed Jun 18 09:34:38 PDT 2003
//    Communicate node numbers.
//
//    Kathleen Bonnell, Wed Jun 18 17:47:10 PDT 2003  
//    When telling downstream items that ghost zones are present, use
//    AVT_CREATED_GHOSTS to distinguish between those designated by the file
//    format (AVT_HAS_GHOSTS) and those created here. (Needed currently so
//    that Pick can return the correct cell id and/or coords).
//
// ****************************************************************************

bool
avtGenericDatabase::CommunicateGhosts(avtDatasetCollection &ds, 
   vector<int> &doms, avtDataSpecification_p &spec, avtSourceFromDatabase *src)
{
    bool rv = false;
    void_ref_ptr vr = cache.GetVoidRef("any_mesh",
                                   AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                                  -1, -1);
    if (*vr != NULL)
    {
        int  i, j, k;
        avtDomainBoundaries *dbi = (avtDomainBoundaries*)*vr;

        //
        // Make sure that this mesh is the mesh we have boundary information
        // for.
        //
        vector<vtkDataSet *> confirmlist;
        for (i = 0 ; i < doms.size() ; i++)
        {
            confirmlist.push_back(ds.GetDataset(i, 0));
        }
        bool haveRightMesh = dbi->ConfirmMesh(doms, confirmlist);
        int  shouldStop = (haveRightMesh ? 0 : 1);

#ifdef PARALLEL
        int  parallelShouldStop;
        MPI_Allreduce(&shouldStop, &parallelShouldStop, 1, MPI_INT, MPI_MAX,
                      MPI_COMM_WORLD);
        shouldStop = parallelShouldStop;
#endif

        if (shouldStop > 0)
        {
            debug1 << "Not applying ghost zones because the boundary "
                   << "information does not apply to this mesh." << endl;
            return false;
        }

        //
        //  Setup
        //
        int timerHandle = visitTimer->StartTimer();

        // Setup progress
        int   localstage;
        int   nlocalstage;
        char  progressString[1024] = "Calculating ghost zones";
        src->DatabaseProgress(0, 0, progressString);

        src->DatabaseProgress(0, 100,
                         "Calculating ghost zones: waiting for all MPI tasks");
#ifdef PARALLEL
        Barrier();
#endif
        // Setup materials
        int anymats    = false;
        int allmats    = true; 
        int most_mixvars = 0;
        const int didnt_get_any = 100;
        int least_mixvars = didnt_get_any; 
        for (i = 0 ; i < doms.size() ; i++)
        {
            avtMaterial *mat = ds.GetMaterial(i);
            if (mat)
                anymats = true;
            else
                allmats = false;

            if (mat != NULL && mat->GetMixlen() > 0)
            {
                int num = ds.GetAllMixVars(i).size();
                most_mixvars = (most_mixvars > num ? most_mixvars : num);
                least_mixvars = (least_mixvars < num ? least_mixvars : num);
            }
        }

#ifdef PARALLEL
        int anymats_tmp    = anymats;
        int allmats_tmp    = allmats; 
        int least_mix_tmp  = least_mixvars;
        int most_mix_tmp   = most_mixvars;
        MPI_Allreduce(&anymats_tmp, &anymats,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);
        MPI_Allreduce(&allmats_tmp, &allmats,1,MPI_INT,MPI_MIN,MPI_COMM_WORLD);
        MPI_Allreduce(&most_mix_tmp, &most_mixvars, 1, MPI_INT, MPI_MAX,
                      MPI_COMM_WORLD);
        MPI_Allreduce(&least_mix_tmp, &least_mixvars, 1, MPI_INT, MPI_MIN,
                      MPI_COMM_WORLD);
#endif

        if (anymats && !allmats)
            EXCEPTION1(VisItException, "Material selection must be performed "
                       "on all domains.  Please see a VisIt developer.");
        if (least_mixvars != didnt_get_any)
        {
            if (most_mixvars != least_mixvars)
            {
                debug1 << "Not all of the domains have the same number of "
                       << "mixvars." << endl;
                debug1 << "Most_mixvars = " << most_mixvars << endl;
                debug1 << "Least_mixvars = " << least_mixvars << endl;
                EXCEPTION1(VisItException, "Mixed variables must be defined "
                           "on all domains that have mixed materials. "
                           " Please see a VisIt developer.");
            }
        }
        int nummixvars = most_mixvars;

        // Setup vars
        const char *varname = spec->GetVariable();
        int ts = spec->GetTimestep();
        avtVarType type = GetMetaData(ts)->DetermineVarType(varname);

        localstage  = 1;
        nlocalstage = (1 + 
                       ((type==AVT_SCALAR_VAR || type==AVT_VECTOR_VAR) ? 1:0) +
                       (anymats ? 1:0) + 
                       (nummixvars > 0 ? 1:0) +
                       (spec->NeedZoneNumbers()||spec->NeedStructuredIndices()
                                ? 1: 0));

        //
        //  Exchange Meshes
        //
        src->DatabaseProgress(localstage++, nlocalstage,
                              "Creating ghost zones for meshes");

        vector<vtkDataSet *> list;
        for (i = 0 ; i < doms.size() ; i++)
        {
            list.push_back(ds.GetDataset(i, 0));
            list[i]->Register(NULL);
        }
        vector<vtkDataSet *> newList = dbi->ExchangeMesh(doms, list);

        //
        //  Copy FieldData arrays indivdually, as there may be
        //  some already in existence in newList.
        //
        for (i = 0 ; i < doms.size() ; i++)
        {
            vtkFieldData *fd = list[i]->GetFieldData();
            for (k = 0; k < fd->GetNumberOfArrays(); k++)
            {
                newList[i]->GetFieldData()->AddArray(fd->GetArray(k));
            }
            ds.SetDataset(i, 0, newList[i]);
            newList[i]->Delete();
        }

        //
        //  Exchange Variables
        //
        if (type == AVT_SCALAR_VAR || type == AVT_MATSPECIES)
        {
            src->DatabaseProgress(localstage++, nlocalstage,
                                  "Creating ghost zones for scalars");

            avtCentering centering;
            if (type == AVT_MATSPECIES)
                centering = AVT_ZONECENT;
            else
                centering = GetMetaData(ts)->GetScalar(varname)->centering;

            bool isPointData = (centering == AVT_NODECENT ? true : false);
            vector<vtkDataArray *> scalars;
            for (i = 0 ; i < doms.size() ; i++)
            {
                vtkDataSet *ds1 = list[i];
                vtkDataArray *s   = NULL;
                if (centering == AVT_NODECENT)
                {
                    s = ds1->GetPointData()->GetScalars();
                }
                else
                {
                    s = ds1->GetCellData()->GetScalars();
                }
                scalars.push_back(s);
            }
            vector<vtkDataArray *> scalarsOut;
            scalarsOut = dbi->ExchangeScalar(doms, isPointData, scalars);
            for (i = 0 ; i < doms.size() ; i++)
            {
                vtkDataSet *ds1 = ds.GetDataset(i, 0);
                vtkDataArray *s   = scalarsOut[i];
                if (centering == AVT_NODECENT)
                {
                    ds1->GetPointData()->SetScalars(s);
                }
                else
                {
                    ds1->GetCellData()->SetScalars(s);
                }
                s->Delete();
            }
        }

        if (type == AVT_VECTOR_VAR)
        {
            src->DatabaseProgress(localstage++, nlocalstage,
                                  "Creating ghost zones for vectors");

            const avtVectorMetaData *vmd = GetMetaData(ts)->GetVector(varname);
            bool isPointData = (vmd->centering == AVT_NODECENT ? true : false);
            vector<vtkDataArray *> vectors;
            for (i = 0 ; i < doms.size() ; i++)
            {
                vtkDataSet *ds1 = list[i];
                vtkDataArray *s   = NULL;
                if (vmd->centering == AVT_NODECENT)
                {
                    s = ds1->GetPointData()->GetVectors();
                }
                else
                {
                    s = ds1->GetCellData()->GetVectors();
                }
                vectors.push_back(s);
            }
            vector<vtkDataArray *> vectorsOut;
            vectorsOut = dbi->ExchangeFloatVector(doms, isPointData, vectors);
            for (i = 0 ; i < doms.size() ; i++)
            {
                vtkDataSet *ds1 = ds.GetDataset(i, 0);
                vtkDataArray *s   = vectorsOut[i];
                if (vmd->centering == AVT_NODECENT)
                {
                    ds1->GetPointData()->SetVectors(s);
                }
                else
                {
                    ds1->GetCellData()->SetVectors(s);
                }
                s->Delete();
            }
        }

        //
        // Exchange secondary variables.
        //
        const vector<CharStrRef> &var2nd = spec->GetSecondaryVariables();
        avtDatabaseMetaData *metadata = GetMetaData(ts);
        for (i = 0 ; i < var2nd.size() ; i++)
        {
            CharStrRef curVar  = var2nd[i];
            avtVarType varType = metadata->DetermineVarType(*curVar);
            switch (varType)
            {
              case AVT_SCALAR_VAR:
                {
                    avtCentering centering = metadata->GetScalar(*curVar)
                                                                   ->centering;
                    bool isPointData = (centering == AVT_NODECENT
                                                     ? true : false);
                    vector<vtkDataArray *> scalars;
                    for (j = 0 ; j < doms.size() ; j++)
                    {
                        vtkDataSet *ds1 = list[j];
                        vtkDataSetAttributes *atts = NULL;
                        if (isPointData)
                        {
                            atts = ds1->GetPointData();
                        }
                        else
                        {
                            atts = ds1->GetCellData();
                        }
                        scalars.push_back(atts->GetArray(*curVar));
                    }
                    vector<vtkDataArray *> scalarsOut;
                    scalarsOut = dbi->ExchangeScalar(doms,isPointData,scalars);
                    for (j = 0 ; j < doms.size() ; j++)
                    {
                        vtkDataSet *ds1 = ds.GetDataset(j, 0);
                        vtkDataSetAttributes *atts = NULL;
                        if (isPointData)
                        {
                            atts = ds1->GetPointData();
                        }
                        else
                        {
                            atts = ds1->GetCellData();
                        }
                        atts->AddArray(scalarsOut[j]);
                        scalarsOut[j]->Delete();
                    }
                }
                break;
              case AVT_MATSPECIES:
                {
                    vector<vtkDataArray *> scalars;
                    for (j = 0 ; j < doms.size() ; j++)
                    {
                        scalars.push_back(list[j]->GetCellData()->GetArray(
                                                                     *curVar));
                    }
                    vector<vtkDataArray *> scalarsOut;
                    scalarsOut = dbi->ExchangeScalar(doms, false, scalars);
                    for (j = 0 ; j < doms.size() ; j++)
                    {
                        vtkDataSet *ds1 = ds.GetDataset(j, 0);
                        ds1->GetCellData()->AddArray(scalarsOut[j]);
                        scalarsOut[j]->Delete();
                    }
                }
                break;
              case AVT_VECTOR_VAR:
                {
                    avtCentering centering = metadata->GetVector(*curVar)
                                                                   ->centering;
                    bool isPointData = (centering == AVT_NODECENT
                                                     ? true : false);
                    vector<vtkDataArray *> vectors;
                    for (j = 0 ; j < doms.size() ; j++)
                    {
                        vtkDataSet *ds1 = list[j];
                        vtkDataSetAttributes *atts = NULL;
                        if (isPointData)
                        {
                            atts = ds1->GetPointData();
                        }
                        else
                        {
                            atts = ds1->GetCellData();
                        }
                        vectors.push_back(atts->GetArray(*curVar));
                    }
                    vector<vtkDataArray *> vectorsOut;
                    vectorsOut = dbi->ExchangeFloatVector(doms,isPointData,
                                                          vectors);
                    for (j = 0 ; j < doms.size() ; j++)
                    {
                        vtkDataSet *ds1 = ds.GetDataset(j, 0);
                        vtkDataSetAttributes *atts = NULL;
                        if (isPointData)
                        {
                            atts = ds1->GetPointData();
                        }
                        else
                        {
                            atts = ds1->GetCellData();
                        }
                        atts->AddArray(vectorsOut[j]);
                        vectorsOut[j]->Delete();
                    }
                }
                break;
              default:
                EXCEPTION1(VisItException, "Cannot exchange secondary "
                         "variables that aren't scalars, vectors, or species");
            }
        }

        //
        //  Exchange Materials and Mixed Variables
        //
        if (anymats)
        {
            src->DatabaseProgress(localstage++, nlocalstage,
                                  "Creating ghost zones for materials");

            // materials
            vector<avtMaterial*> matList;
            for (i = 0 ; i < doms.size() ; i++)
                matList.push_back(ds.GetMaterial(i));

            vector<avtMaterial*> newMatList = dbi->ExchangeMaterial(doms,
                                                                    matList);

            for (i = 0 ; i < doms.size() ; i++)
                ds.SetMaterial(i, newMatList[i]);

            // mixvars
            if (nummixvars > 0)
            {
                src->DatabaseProgress(localstage++, nlocalstage,
                                      "Creating ghost zones for mixed vars");
                for (i = 0 ; i < nummixvars ; i++)
                {
                    vector<avtMixedVariable*> mixvarList;
                    int numDomains = doms.size();
                    for (j = 0 ; j < numDomains ; j++)
                    {
                        avtMaterial *mat = matList[j];
                        if (mat != NULL && mat->GetMixlen() > 0)
                        {
                            avtMixedVariable *mv = (avtMixedVariable *)
                                                     *(ds.GetAllMixVars(j)[i]);
                            mixvarList.push_back(mv);
                        }
                        else
                        {
                            mixvarList.push_back(NULL);
                        }
                    }

                    vector<avtMixedVariable*> newMixvarList = 
                        dbi->ExchangeMixVar(doms, matList, mixvarList);

                    for (j = 0 ; j < doms.size() ; j++)
                        if (newMixvarList[j] != NULL)
                        {
                            void_ref_ptr vr = void_ref_ptr(newMixvarList[j],
                                                  avtMixedVariable::Destruct);
                            ds.ReplaceMixVar(j, vr);
                        }
                }
            }
        }

        //
        // Exchange OriginalNodes Arrays.
        //
        if (spec->NeedNodeNumbers())
        {
            vector<vtkDataArray *> nodeNums;
            for (j = 0 ; j < doms.size() ; j++)
            {
                vtkDataSet *ds1 = list[j];
                nodeNums.push_back(ds1->GetPointData()->GetArray(
                                                    "avtOriginalNodeNumbers"));
            }
            vector<vtkDataArray *> nodeNumsOut;
            nodeNumsOut = dbi->ExchangeIntVector(doms,true,nodeNums);
            for (j = 0 ; j < doms.size() ; j++)
            {
                vtkDataSet *ds1 = ds.GetDataset(j, 0);
                ds1->GetPointData()->AddArray(nodeNumsOut[j]);
                nodeNumsOut[j]->Delete();
            }
        }

        //
        // Exchange OriginalCells Arrays.
        //
        if (spec->NeedZoneNumbers() || spec->NeedStructuredIndices())
        {
            vector<vtkDataArray *> cellNums;
            for (j = 0 ; j < doms.size() ; j++)
            {
                vtkDataSet *ds1 = list[j];
                cellNums.push_back(ds1->GetCellData()->GetArray(
                                                    "avtOriginalCellNumbers"));
            }
            vector<vtkDataArray *> cellNumsOut;
            cellNumsOut = dbi->ExchangeIntVector(doms,false,cellNums);
            for (j = 0 ; j < doms.size() ; j++)
            {
                vtkDataSet *ds1 = ds.GetDataset(j, 0);
                ds1->GetCellData()->AddArray(cellNumsOut[j]);
                cellNumsOut[j]->Delete();
            }
        }

        //
        // We added references to the mesh with no ghost zones so that we could
        // reference it through the communication.  Now that we are done,
        // remove those references.
        //
        for (i = 0 ; i < doms.size() ; i++)
        {
            list[i]->Delete();
        }

        //
        // Tell everything downstream that we do have ghost zones.
        //
        avtDatabaseMetaData *md = GetMetaData(ts);
        string meshname = md->MeshForVar(spec->GetVariable());
        GetMetaData(ts)->SetContainsGhostZones(meshname, AVT_CREATED_GHOSTS);

        src->DatabaseProgress(1, 0, progressString);
        rv = true;
        visitTimer->StopTimer(timerHandle, "Creating ghost zones");
    }

    return rv;
}

// ****************************************************************************
//  Method: avtGenericDatabase::ApplyGhostForDomainNesting
//
//  Purpose:
//      When domains nest within each other (as for an AMR mesh), this method
//      will apply the "vtkGhostLevels" label to any zones in coarser domains 
//      that have finer domains in the current selection.
//
//      Note: In addition to knowing the list of domains being processed on
//      this processor, we also need to know the list of domains being
//      processed on all processors. The fact is, this processor may contain
//      a coarse domain that should have zones ghosted out due to a finer
//      patch in the current selection but living on another processor 
//
//  Arguments:
//      ds        The dataset collection.
//      doms      A list of domains.
//      spec      A data specification.
//
//  Returns:      True if any zones where ghosted.  False otherwise.
//
//  Programmer:   Mark C. Miller
//  Creation:     October 13, 2003
//
// ****************************************************************************

bool
avtGenericDatabase::ApplyGhostForDomainNesting(avtDatasetCollection &ds, 
   vector<int> &doms, vector<int> &allDoms, avtDataSpecification_p &spec)
{
    bool rv = false;

    void_ref_ptr vr = cache.GetVoidRef("any_mesh",
                                   AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                                  spec->GetTimestep(), -1);

    if (*vr != NULL)
    {
        int  i;
        avtDomainNesting *dn = (avtDomainNesting*)*vr;

        vector<vtkDataSet *> list;
        for (i = 0 ; i < doms.size() ; i++)
        {
            list.push_back(ds.GetDataset(i, 0));
            list[i]->Register(NULL);
        }

        rv = dn->ApplyGhost(doms, allDoms, list);

        //
        // Tell everything downstream that we do have ghost zones.
        //
        if (rv)
        {
            int ts = spec->GetTimestep();
            avtDatabaseMetaData *md = GetMetaData(ts);
            string meshname = md->MeshForVar(spec->GetVariable());
            GetMetaData(ts)->SetContainsGhostZones(meshname, AVT_CREATED_GHOSTS);
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::MaterialSelect
//
//  Purpose:
//      Perform a material selection.
//
//  Arguments:
//      ds        The dataset collection.
//      domains   A list of domains to be read.
//      spec      A data specification.
//      src       The source object.
//      didGhost  Indicates whether we created ghosts.
//
//  Programmer:   Hank Childs
//  Creation:     October 26, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Nov 26 09:01:36 PST 2001
//    Account for materials being read in while the mesh is being read in.
//
//    Hank Childs, Thu Jul  4 16:34:33 PDT 2002
//    Account for cases where there are multiple mixed variables.
//
//    Jeremy Meredith, Tue Aug 13 14:39:30 PDT 2002
//    Added the two subdivision flags and set their counterparts
//    in the output avtDataValidity structure.
//
//    Hank Childs, Wed Oct  9 10:16:15 PDT 2002
//    Change test for if we are using all the domains, since it did not work
//    in parallel.
//
//    Jeremy Meredith, Thu Oct 24 15:37:24 PDT 2002
//    Added smoothing option and clean zones only option.
//
//    Hank Childs, Fri Jan 17 09:43:03 PST 2003
//    Make sure to stop the timer if we return early.
//
//    Jeremy Meredith, Wed Mar 19 16:50:58 PST 2003
//    Added support for variables defined on only a subset of the domains.
//
//    Jeremy Meredith, Thu Jun 12 09:09:30 PDT 2003
//    Added a flag for internal boundary surfaces.
//
//    Hank Childs, Tue Jul 22 21:48:09 PDT 2003
//    Send down information about whether or not we communicated ghost zones.
//
//    Hank Childs, Fri Sep 12 16:27:38 PDT 2003
//    Send down info about whether the MIR was forced.
//
//    Jeremy Meredith, Fri Sep  5 15:32:30 PDT 2003
//    Added a flag for the MIR algorithm.
//
// ****************************************************************************

void
avtGenericDatabase::MaterialSelect(avtDatasetCollection &ds,
                            vector<int> &domains, avtDataSpecification_p &spec,
                            avtSourceFromDatabase *src, bool didGhosts)
{
    int i, timerHandle = visitTimer->StartTimer();

    const char *var = spec->GetVariable();
    int         ts  = spec->GetTimestep();

    char  progressString[1024] = "Performing material selection";

    bool shouldDoMatSelect = false;
    for (i = 0 ; i < ds.GetNDomains() ; i++)
    {
        shouldDoMatSelect = shouldDoMatSelect || ds.needsMatSelect[i];
    }
    if (! shouldDoMatSelect)
    {
        visitTimer->StopTimer(timerHandle, "Deciding not to material "
                                           "select data");
        return;
    }

    bool subdivisionOccurred   = false;
    bool notAllCellsSubdivided = false;

    src->DatabaseProgress(0, 0, progressString);

    //
    // If we are communicating ghost zones ourselves, the MIR objects we
    // create could be invalid if we are using a different set of domains.  To
    // be on the safe side, only re-use MIRs if we are re-using all of the 
    // data.  We could cache all of the MIRs and re-use which ever ones make 
    // sense based on which domains are being used, but that could blow the
    // cache pretty quickly.
    //
    bool reUseMIR = true;
    if (didGhosts)
    {
        if (!spec->UsesAllDomains())
        {
            reUseMIR = false;
        }
    }

    for (i = 0 ; i < ds.GetNDomains() ; i++)
    {
        if (ds.GetDataset(i,0)!=NULL  &&  ds.needsMatSelect[i])
        {
            vector<avtMixedVariable *> mvl;
            vector<void_ref_ptr> &vrl = ds.GetAllMixVars(i);
            for (int j = 0 ; j < vrl.size() ; j++)
            {
                avtMixedVariable *mv = (avtMixedVariable *) *vrl[j];
                mvl.push_back(mv);
            }

            bool so;
            bool nacs;
            ds.avtds[i] = MaterialSelect(ds.GetDataset(i,0), ds.GetMaterial(i),
                                mvl, domains[i], var, ts,
                                ds.matnames[i], ds.labels[i],
                                spec->NeedInternalSurfaces(),
                                spec->NeedBoundarySurfaces(),
                                spec->NeedValidFaceConnectivity(),
                                spec->NeedSmoothMaterialInterfaces(),
                                spec->NeedCleanZonesOnly(), 
                                spec->MustDoMaterialInterfaceReconstruction(),
                                spec->UseNewMIRAlgorithm() ? 1 : 0,
                                didGhosts, so, nacs, reUseMIR);

            notAllCellsSubdivided = notAllCellsSubdivided || nacs ||
                (subdivisionOccurred && !so) || (!subdivisionOccurred && so);
            subdivisionOccurred = subdivisionOccurred || so;
        }

        src->DatabaseProgress(i, ds.GetNDomains(), progressString);
    }
    src->DatabaseProgress(1, 0, progressString);
    src->GetOutput()->GetInfo().GetValidity().SetSubdivisionOccurred(
                                                        subdivisionOccurred);
    src->GetOutput()->GetInfo().GetValidity().SetNotAllCellsSubdivided(
                                                        notAllCellsSubdivided);

    visitTimer->StopTimer(timerHandle, "Material selecting dataset");
    visitTimer->DumpTimings();
}


// ****************************************************************************
//  Method: avtGenericDatabase::CreateOriginalZones
//
//  Purpose:
//    Create original zones array. 
//
//  Arguments:
//      ds        The dataset collection.
//      src       The source object.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 12, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar 26 15:58:05 PST 2002
//    Make this method keep track of a starting cell number for each data set.
//
//    Kathleen Bonnell, Mon May 20 17:01:31 PDT 2002  
//    Removed starting cell number.  Pass domain number to 
//    AddOriginalCellsArray.  
//    
// ****************************************************************************

void
avtGenericDatabase::CreateOriginalZones(avtDatasetCollection &ds, 
                              vector<int> &domains, avtSourceFromDatabase *src)
{
    char  progressString[1024] = "Creating Original Zones Array";
    src->DatabaseProgress(0, 0, progressString);
    for (int i = 0 ; i < ds.GetNDomains() ; i++)
    {
        AddOriginalCellsArray(ds.GetDataset(i, 0), domains[i]);
        src->DatabaseProgress(i, ds.GetNDomains(), progressString);
    }
    src->DatabaseProgress(1, 0, progressString);
}


// ****************************************************************************
//  Method: avtGenericDatabase::CreateOriginalNodes
//
//  Purpose:
//    Create original nodes array. 
//
//  Arguments:
//      ds        The dataset collection.
//      src       The source object.
//
//  Programmer:   Hank Childs 
//  Creation:     June 18, 2003
//
// ****************************************************************************

void
avtGenericDatabase::CreateOriginalNodes(avtDatasetCollection &ds, 
                              vector<int> &domains, avtSourceFromDatabase *src)
{
    char  progressString[1024] = "Creating Original Nodes Array";
    src->DatabaseProgress(0, 0, progressString);
    for (int i = 0 ; i < ds.GetNDomains() ; i++)
    {
        AddOriginalNodesArray(ds.GetDataset(i, 0), domains[i]);
        src->DatabaseProgress(i, ds.GetNDomains(), progressString);
    }
    src->DatabaseProgress(1, 0, progressString);
}


// ****************************************************************************
//  Method: avtGenericDatabase::CreateStructuredIndices
//
//  Purpose:
//      Create an array that contains the index number of each cell.
//
//  Arguments:
//      ds        The dataset collection.
//      src       The source object.
//
//  Programmer:   Hank Childs 
//  Creation:     September 30, 2002
//
// ****************************************************************************

void
avtGenericDatabase::CreateStructuredIndices(avtDatasetCollection &dsc, 
                                            avtSourceFromDatabase *src)
{
    char  progressString[1024] = "Creating Structured Indices";
    src->DatabaseProgress(0, 0, progressString);
    for (int i = 0 ; i < dsc.GetNDomains() ; i++)
    {
        vtkDataSet *ds = dsc.GetDataset(i, 0);
        int dstype = ds->GetDataObjectType();
        int dims[3];
        if (dstype == VTK_RECTILINEAR_GRID)
        {
            vtkRectilinearGrid *rgrid = (vtkRectilinearGrid *) ds;
            rgrid->GetDimensions(dims);
        }
        else if (dstype == VTK_STRUCTURED_GRID)
        {
            vtkStructuredGrid *sgrid = (vtkStructuredGrid *) ds;
            sgrid->GetDimensions(dims);
        }
        else
        {
            debug1 << "Requested structured indices for non-structured grid."
                   << endl;
            continue;
        }
        vtkUnsignedIntArray *orig_dims = vtkUnsignedIntArray::New();
        orig_dims->SetNumberOfTuples(3);
        orig_dims->SetValue(0, dims[0]);
        orig_dims->SetValue(1, dims[1]);
        orig_dims->SetValue(2, dims[2]);
        orig_dims->SetName("avtOriginalStructuredDimensions");
        ds->GetFieldData()->AddArray(orig_dims);
        orig_dims->Delete();
        AddOriginalCellsArray(ds, -1);
        src->DatabaseProgress(i, dsc.GetNDomains(), progressString);
    }
    src->DatabaseProgress(1, 0, progressString);
}


// ****************************************************************************
//  Method: avtGenericDatabase::NumStagesForFetch
//
//  Purpose:
//      This returns how many stages there are for the fetch.  Some databases
//      have more than one stage (read from file format, perform material
//      selection, etc).
//
//  Arguments:
//      spec    The data specification that will be sent in.
//
//  Returns:    A good default number of stages (1).
//
//  Programmer: Hank Childs
//  Creation:   October 26, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Dec 19 20:14:30 PST 2001
//    Added a stage for species selection.
//
//    Hank Childs, Mon Sep 30 18:01:26 PDT 2002
//    Added stages for creating the original zones and structured indices
//    arrays.
//
//    Hank Childs, Fri Nov 22 16:39:13 PST 2002
//    Use the SIL restriction traverser since SIL restriction routines were
//    antiquated.
//
//    Hank Childs, Wed Jun 18 09:34:38 PDT 2003
//    Added a stage for needing node numbers.
//
//    Hank Childs, Wed Aug 13 08:09:20 PDT 2003
//    We should do MIR if we are told to by the data spec, not if the type of
//    variable is material.
//
//    Jeremy Meredith, Wed Aug 20 09:56:19 PDT 2003
//    Refactored MIR into a base and subclass.  This will allow us to swap
//    in other MIR algorithms more easily.
//
// ****************************************************************************

int
avtGenericDatabase::NumStagesForFetch(avtDataSpecification_p spec)
{
    int numStages = 1;   // Always one for I/O.

    avtSILRestriction_p silr = spec->GetRestriction();
    avtSILRestrictionTraverser trav(silr);
    vector<int> domains;
    trav.GetDomainList(domains);
    
    //
    // Determine if there will be a material selection phase.
    //
    bool needMatSel = false;
    
    needMatSel |= spec->MustDoMaterialInterfaceReconstruction();
    if (!needMatSel)
    {
        for (int i = 0 ; i < domains.size() ; i++)
        {
            bool needMatSelForThisDom;
            trav.GetMaterials(domains[i], needMatSelForThisDom);
            if (needMatSelForThisDom)
            {
                needMatSel = true;
                break;
            }
        }
    }

    //
    // Some file formats do their own selection.  They have a combined stage,
    // so do not add one in that case.
    //
    if (needMatSel && !Interface->PerformsMaterialSelection())
    {
        numStages += 1;
    }

    //
    // Now determine if there will be a phase to communicate ghost zones.
    //
    void_ref_ptr vr = cache.GetVoidRef("any_mesh",
                                  AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
                                  -1, -1);
    if (*vr != NULL && trav.UsesAllData())
    {
        numStages += 1;
    }

    vector<bool> list;
    if (trav.GetSpecies(list))
    {
        numStages += 1;
    }

    if (spec->NeedNodeNumbers())
    {
        numStages += 1;
    }

    if (spec->NeedZoneNumbers())
    {
        numStages += 1;
    }
  
    if (spec->NeedStructuredIndices())
    {
        numStages += 1;
    }

    return numStages;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryScalars
//
//  Purpose:
//    Queries the db regarding scalar var info for a specific cell or nodes. 
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    zone        The zone to query.
//    ts          The timestep to query.
//    nodes       The nodes to query.
//    varInfo     A place to store the results. 
//    zonePick    Whether or not the pick was a zone pick.
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     June 27, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Made filling of values contigent upon it not being filled already.
//    Don't set variable name, it should be set already.
//
//    Kathleen Bonnell, Fri Jun 20 13:57:30 PDT 2003  
//    Add support for node-pick. 
//
//    Mark C. Miller, 30Sep03 added timestep argument
//    
// ****************************************************************************

bool
avtGenericDatabase::QueryScalars(const std::string &varName, const int dom, 
                                 const int element, const int ts, 
                                 const std::vector<int> &incidentElements, 
                                 PickVarInfo &varInfo, const bool zonePick)
{
    bool rv = false;
    int i;
    if (varInfo.GetValues().empty())
    {
        std::vector<double> vals;
        std::vector<std::string> names;
        const avtScalarMetaData *smd = GetMetaData(ts)->GetScalar(varName);
        if (!smd)
        {
             debug5 << "Querying scalar var, but could not retrieve"
                    << " meta data!" << endl;
             return false;
        }
        char temp[80];
        vtkDataArray *scalars = GetScalarVariable(varName.c_str(), ts, dom, "_all");
        if (scalars) 
        {
            bool zoneCent, validCentering = true;
            if (smd->centering == AVT_NODECENT)
            {
                varInfo.SetCentering(PickVarInfo::Nodal);
                zoneCent = false; 
            }
            else if (smd->centering == AVT_ZONECENT)
            {
                varInfo.SetCentering(PickVarInfo::Zonal);
                zoneCent = true; 
            }
            else 
            {
                validCentering = false; 
            }

            if (validCentering)
            {
                if (zoneCent != zonePick) 
                {
                    // the info we're after is associated with incidentElements
                    for (i = 0; i < incidentElements.size(); i++)
                    {
                        sprintf(temp, "(%d)", incidentElements[i]);
                        names.push_back(temp);
                        vals.push_back(scalars->GetTuple1(incidentElements[i]));
                    }
                }
                else 
                {
                    // the info we're after is associated with element
                    sprintf(temp, "(%d)", element);
                    names.push_back(temp);
                    vals.push_back(scalars->GetTuple1(element));
                }
            }
        }
        if (!vals.empty())
        {
            varInfo.SetNames(names);
            varInfo.SetValues(vals);
            vals.clear();
            names.clear();
            rv = true;
        }
    }

 
    //
    //  If this is a mixed-var, then get the material values.
    //
    void_ref_ptr vr = cache.GetVoidRef(varName.c_str(), 
                                AUXILIARY_DATA_MIXED_VARIABLE, 
                                ts, dom); 

    if (*vr != NULL)
    {
        avtMixedVariable *mv = (avtMixedVariable*)(*vr);
        avtMaterial *mat = GetMaterial(dom, varName.c_str(), ts);
        std::vector<CellMatInfo> matInfo; 
        std::vector<std::string> mN;
        std::vector<double> mV;
        std::vector<int> nMats;
        int i, j, nMatsPerZone;
        bool mixed = false;
        
        if (zonePick)
        { 
            // check that zones reported by pick are accurate for retrieving
            // material var info
            if (element < 0 || element >= mat->GetNZones())
            {
                debug5 << "CANNOT QUERY MATFRACS, ZONE IS OUT OF RANGE" << endl;
                return rv;
            }
            matInfo = mat->ExtractCellMatInfo(element);
            nMatsPerZone = 0;
            for (int i = 0; i < matInfo.size(); i++)
            { 
                if (matInfo[i].mix_index != -1)
                {
                    mixed = true;
                    mN.push_back(matInfo[i].name);
                    mV.push_back(mv->GetBuffer()[matInfo[i].mix_index]);
                    nMatsPerZone++;
                }
            }
            nMats.push_back(nMatsPerZone);
        }
        else
        {
            // check that zones reported by pick are accurate for retrieving
            // material var info
            bool zonesInRange = true;
            int nmatzones = mat->GetNZones();
            for (i = 0; i < incidentElements.size(); i++)
            {
                if (incidentElements[i] < 0 || incidentElements[i] >= nmatzones)
                {
                    zonesInRange = false;
                    break; 
                }
            }
            if (!zonesInRange)
            {
                debug5 << "CANNOT QUERY MATFRACS ZONE IS OUT OF RANGE" << endl;
                return rv;
            }
            for (j = 0; j < incidentElements.size(); j++)
            {
                nMatsPerZone = 0;
                matInfo = mat->ExtractCellMatInfo(incidentElements[j]);
                for (i = 0; i < matInfo.size(); i++)
                { 
                    if (matInfo[i].mix_index != -1)
                    {
                        mixed = true;
                        mN.push_back(matInfo[i].name);
                        mV.push_back(mv->GetBuffer()[matInfo[i].mix_index]);
                        nMatsPerZone++;
                    }
                }
                nMats.push_back(nMatsPerZone);
            }
        }
        if (mixed)
        {
            varInfo.SetMixVar(true);
            varInfo.SetMixNames(mN);
            varInfo.SetMixValues(mV);
            varInfo.SetNumMatsPerZone(nMats);
        }
        rv = true;
    }

    // 
    // This is where we could allow the interface to add more information.
    // 
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryVectors
//
//  Purpose:
//    Queries the db regarding vector var info for a specific cell or nodes.  
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    zone        The zone to query.
//    ts          The timestep to query.
//    nodes       The nodes to query.
//    varInfo     A place to store the results. 
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     June 27, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Made filling of values contigent upon it not being filled already.
//    Don't set variable name, it should be set already.
//
//    Kathleen Bonnell, Fri Jun 20 13:57:30 PDT 2003  
//    Add support for node-pick. 
//    
// ****************************************************************************

bool
avtGenericDatabase::QueryVectors(const std::string &varName, const int dom, 
                                 const int element, const int ts, 
                                 const std::vector<int> &incidentElements, 
                                 PickVarInfo &varInfo, const bool zonePick)
{
    bool rv = false;
    if (varInfo.GetValues().empty())
    {
        const avtVectorMetaData *vmd = GetMetaData(ts)->GetVector(varName);
        if (!vmd)
        {
            debug5 << "Querying vector var, but could not retrieve"
                   << " meta data!" << endl;
            return false;
        }
        std::vector<std::string> names;
        std::vector<double> vals;
        char buff[80];
        vtkDataArray *vectors = GetVectorVariable(varName.c_str(), ts, dom, "_all");
        int nComponents = 0;; 
        double *temp = NULL; 
        double mag = 0.;
        if (vectors)
        {
            bool zoneCent, validCentering = true;
            if (vmd->centering == AVT_NODECENT)
            {
                varInfo.SetCentering(PickVarInfo::Nodal);
                zoneCent = false;
            }
            else if (vmd->centering == AVT_ZONECENT)
            {
                varInfo.SetCentering(PickVarInfo::Zonal);
                zoneCent = true;
            }
            else 
            {
                validCentering = false;
            }
            if (validCentering)
            {
                nComponents = vectors->GetNumberOfComponents();
                temp = new double[nComponents];
                if (zonePick != zoneCent) 
                { 
                    // info we're after is associated with incidentElements
                    for (int k = 0; k < incidentElements.size(); k++)
                    {
                        sprintf(buff, "(%d)", incidentElements[k]); 
                        names.push_back(buff); 
                        vectors->GetTuple(incidentElements[k], temp);
                        mag = 0.;
                        for (int i = 0; i < nComponents; i++)
                        {
                            vals.push_back(temp[i]);
                            mag += (temp[i] * temp[i]); 
                        }
                        mag = sqrt(mag);
                        vals.push_back(mag);
                    }
                }
                else 
                {
                    // info we're after is associated with element 
                    sprintf(buff, "(%d)", element);
                    names.push_back(buff); 
                    vectors->GetTuple(element, temp);
                    mag = 0.;
                    for (int i = 0; i < nComponents; i++)
                    {
                        vals.push_back(temp[i]);
                        mag +=  (temp[i] * temp[i]); 
                    }
                    mag = sqrt(mag);
                    vals.push_back(mag);
                }
                delete [] temp;
            }
        }
        if (!vals.empty())
        {
            varInfo.SetNames(names);
            varInfo.SetValues(vals);
            vals.clear();
            names.clear();
            rv = true;
        }
    }
    // 
    // This is where we could allow the interface to add more information.
    // 
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryTensors
//
//  Purpose:
//    Queries the db regarding tensor var info for a specific cell or nodes.  
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    zone        The zone to query.
//    ts          The timestep to query.
//    nodes       The nodes to query.
//    varInfo     A place to store the results. 
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Hank Childs
//  Creation:     September 22, 2003
//
// ****************************************************************************

bool
avtGenericDatabase::QueryTensors(const std::string &varName, const int dom, 
                                 const int element, const int ts, 
                                 const std::vector<int> &incidentElements, 
                                 PickVarInfo &varInfo, const bool zonePick)
{
    bool rv = false;
    if (varInfo.GetValues().empty())
    {
        const avtTensorMetaData *tmd = GetMetaData(ts)->GetTensor(varName);
        if (!tmd)
        {
            debug5 << "Querying tensor var, but could not retrieve"
                   << " meta data!" << endl;
            return false;
        }

        std::vector<std::string> names;
        std::vector<double> vals;
        char buff[80];
        vtkDataArray *tensors = GetTensorVariable(varName.c_str(), ts, dom,
                                                  "_all");
        int nComponents = 0;; 
        double *temp = NULL; 
        if (tensors)
        {
            bool zoneCent, validCentering = true;
            if (tmd->centering == AVT_NODECENT)
            {
                varInfo.SetCentering(PickVarInfo::Nodal);
                zoneCent = false;
            }
            else if (tmd->centering == AVT_ZONECENT)
            {
                varInfo.SetCentering(PickVarInfo::Zonal);
                zoneCent = true;
            }
            else 
            {
                validCentering = false;
            }
            if (validCentering)
            {
                nComponents = tensors->GetNumberOfComponents();
                temp = new double[nComponents];
                if (zonePick != zoneCent) 
                { 
                    // info we're after is associated with incidentElements
                    for (int k = 0; k < incidentElements.size(); k++)
                    {
                        sprintf(buff, "(%d)", incidentElements[k]); 
                        names.push_back(buff); 
                        tensors->GetTuple(incidentElements[k], temp);
                        for (int i = 0; i < nComponents; i++)
                            vals.push_back(temp[i]);
                    }
                }
                else 
                {
                    // info we're after is associated with element 
                    sprintf(buff, "(%d)", element);
                    names.push_back(buff); 
                    tensors->GetTuple(element, temp);
                    for (int i = 0; i < nComponents; i++)
                        vals.push_back(temp[i]);
                }
                delete [] temp;
            }
        }
        if (!vals.empty())
        {
            varInfo.SetNames(names);
            varInfo.SetValues(vals);
            vals.clear();
            names.clear();
            rv = true;
        }
    }
    // 
    // This is where we could allow the interface to add more information.
    // 
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QuerySymmetricTensors
//
//  Purpose:
//    Queries the db regarding tensor var info for a specific cell or nodes.  
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    zone        The zone to query.
//    ts          The timestep to query.
//    nodes       The nodes to query.
//    varInfo     A place to store the results. 
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Hank Childs
//  Creation:     September 22, 2003
//
// ****************************************************************************

bool
avtGenericDatabase::QuerySymmetricTensors(const std::string &varName,
                                 const int dom, const int element,const int ts,
                                 const std::vector<int> &incidentElements,
                                 PickVarInfo &varInfo, const bool zonePick)
{
    bool rv = false;
    if (varInfo.GetValues().empty())
    {
        const avtSymmetricTensorMetaData *tmd 
                                       = GetMetaData(ts)->GetSymmTensor(varName);
        if (!tmd)
        {
            debug5 << "Querying tensor var, but could not retrieve"
                   << " meta data!" << endl;
            return false;
        }

        std::vector<std::string> names;
        std::vector<double> vals;
        char buff[80];
        vtkDataArray *tensors = GetSymmetricTensorVariable(varName.c_str(), ts,
                                                           dom, "_all");
        int nComponents = 0;; 
        if (tensors)
        {
            bool zoneCent, validCentering = true;
            if (tmd->centering == AVT_NODECENT)
            {
                varInfo.SetCentering(PickVarInfo::Nodal);
                zoneCent = false;
            }
            else if (tmd->centering == AVT_ZONECENT)
            {
                varInfo.SetCentering(PickVarInfo::Zonal);
                zoneCent = true;
            }
            else 
            {
                validCentering = false;
            }
            if (validCentering)
            {
                nComponents = tensors->GetNumberOfComponents();
                double *temp = new double[nComponents];
                if (zonePick != zoneCent) 
                { 
                    // info we're after is associated with incidentElements
                    for (int k = 0; k < incidentElements.size(); k++)
                    {
                        sprintf(buff, "(%d)", incidentElements[k]); 
                        names.push_back(buff); 
                        tensors->GetTuple(incidentElements[k], temp);
                        for (int i = 0; i < nComponents; i++)
                            vals.push_back(temp[i]);
                    }
                }
                else 
                {
                    // info we're after is associated with element 
                    sprintf(buff, "(%d)", element);
                    names.push_back(buff); 
                    tensors->GetTuple(element, temp);
                    for (int i = 0; i < nComponents; i++)
                        vals.push_back(temp[i]);
                }
                delete [] temp;
            }
        }
        if (!vals.empty())
        {
            varInfo.SetNames(names);
            varInfo.SetValues(vals);
            vals.clear();
            names.clear();
            rv = true;
        }
    }
    // 
    // This is where we could allow the interface to add more information.
    // 
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryMaterial
//
//  Purpose:
//    Queries the db regarding material var info for a specific cell or nodes.  
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    zone        The zone to query.
//    ts          The timestep to query.
//    nodes       The nodes to query.
//    varInfo     A place to store the results. 
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     June 27, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Don't set variable name, it should be set already.
//
//    Kathleen Bonnell, Fri Dec  6 12:10:04 PST 2002   
//    Added test for zone out of range in regards to material zones. 
//
//    Kathleen Bonnell, Fri Jun 20 13:57:30 PDT 2003  
//    Add support for node-pick. 
//    
// ****************************************************************************

bool
avtGenericDatabase::QueryMaterial(const std::string &varName, const int dom, 
                                  const int element, const int ts, 
                                  const std::vector<int> &incidentElements, 
                                  PickVarInfo &varInfo, const bool zonePick)
{
    std::vector<double> volFracs;
    std::vector<std::string> matNames;
    std::vector<std::string> zoneNames;
    std::vector<int> nMats;
    std::vector<CellMatInfo> matInfo; 
    avtMaterial *mat = GetMaterial(dom, varName.c_str(), ts);
    int i, j;
    if (mat == NULL)
    {
        return false;
    }

    char buff[80];
    int numMatsThisZone;
    if (zonePick) // zone number is stored in element
    {
        if (element < 0 || element >= mat->GetNZones())
        {
            debug5 << "CANNOT QUERY MATERIALS ZONE IS OUT OF RANGE" << endl;
            return false;
        }
        numMatsThisZone = 0;
        matInfo = mat->ExtractCellMatInfo(element);
        for (i = 0; i < matInfo.size(); i++)
        {
            if (matInfo[i].vf > 0.)
            {
                matNames.push_back(matInfo[i].name);
                volFracs.push_back(matInfo[i].vf);
                numMatsThisZone++;
            }
        }
        nMats.push_back(numMatsThisZone);
    }
    else // zones are stored in incidentElements.
    {
        bool zonesInRange = true;
        int nmatzones = mat->GetNZones();
        for (i = 0; i < incidentElements.size(); i++)
        {
            if (incidentElements[i] < 0 || incidentElements[i] >= nmatzones)
            {
                zonesInRange = false;
                break; 
            }
        }
        if (!zonesInRange)
        {
            debug5 << "CANNOT QUERY MATERIALS ZONE IS OUT OF RANGE" << endl;
            return false;
        }
        for (j = 0; j < incidentElements.size(); j++)
        {
            numMatsThisZone = 0;
            sprintf(buff, "(%d)", incidentElements[j]);
            zoneNames.push_back(buff);
            matInfo = mat->ExtractCellMatInfo(incidentElements[j]);
            for (i = 0; i < matInfo.size(); i++)
            {
                if (matInfo[i].vf > 0.)
                {
                    matNames.push_back(matInfo[i].name);
                    volFracs.push_back(matInfo[i].vf);
                    numMatsThisZone++;
                }
            }
            nMats.push_back(numMatsThisZone);
        }
    }
    varInfo.SetCentering(PickVarInfo::None);
    varInfo.SetVarIsMaterial(true);
    varInfo.SetNames(zoneNames);
    varInfo.SetMixNames(matNames);
    varInfo.SetMixValues(volFracs);
    varInfo.SetNumMatsPerZone(nMats);

    if (!matNames.empty())
        matNames.clear();
    if (!volFracs.empty())
        volFracs.clear();
    if (!zoneNames.empty())
        zoneNames.clear();
    if (!nMats.empty())
        nMats.clear();

    // 
    // This is where we could allow the interface to add more information.
    // 
    return true;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryNodes
//
//  Purpose:
//    Queries the db regarding material var info for a specific cell or nodes.  
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    zone        The zone to query.
//    ts          The timestep to query.
//    nodes       A place to store the nodes.
//    ppt         A place to store the 'real' picked point for point meshes.
//    useNodeCoords Whether or not to fill in the node coordinates.
//    logical     Whether or not to use logical indices for the node coordinates.
//    nCoords     A place to store the node coordinates.
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     December 6, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 27 14:09:40 PST 2002
//    Added useNodeCoords && nodeCoords arguments.  Fill in nodeCoords as
//    needed.
//
//    Kathleen Bonnell, Tue Sep 16 13:33:30 PDT 2003 
//    Use "base_index" if available when creating logical coords. 
//    
// ****************************************************************************

bool
avtGenericDatabase::QueryNodes(const std::string &varName, const int dom, 
                               const int zone, const int ts, 
                               std::vector<int> &nodes, float ppt[3],
                               const int dim, const bool useNodeCoords, 
                               const bool logical, 
                               std::vector<std::string> &nCoords)
{
    string meshName = GetMetaData(ts)->MeshForVar(varName);
    vtkDataSet *ds = GetMeshDataset(meshName.c_str(), ts, dom, "_all");
    bool rv = false; 
    if (ds)
    {
        vtkIdList *ptIds = vtkIdList::New();
        ds->GetCellPoints(zone, ptIds);
        float coord[3];
        int ijk[3];
        char buff[80];
        int type = ds->GetDataObjectType();
        vtkIntArray *bi = (vtkIntArray*)ds->GetFieldData()->GetArray("base_index");
        int base[3] = {0, 0, 0};
        if (bi)
        {
            base[0] = bi->GetValue(0);
            base[1] = bi->GetValue(1);
            base[2] = bi->GetValue(2);
        }
        for (int i = 0; i < ptIds->GetNumberOfIds(); i++)
        {
            nodes.push_back(ptIds->GetId(i));
            if (useNodeCoords)
            {
                if (logical && (type == VTK_RECTILINEAR_GRID ||
                                type == VTK_STRUCTURED_GRID ))
                {
                    vtkVisItUtility::GetLogicalIndices(ds, false, ptIds->GetId(i), ijk);
                    ijk[0] += base[0];
                    ijk[1] += base[1];
                    ijk[2] += base[2];
                    if (dim == 2)
                    {
                        sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
                    }
                    else 
                    {
                        sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                    }
                }
                else
                {
                    ds->GetPoint(ptIds->GetId(i), coord);
                    if (dim == 2)
                    {
                        sprintf(buff, "<%g, %g>", coord[0], coord[1]);
                    }
                    else 
                    {
                        sprintf(buff, "<%g, %g, %g>", coord[0], coord[1], coord[2]);
                    }
                }
                nCoords.push_back(buff);
            }
        }
        ptIds->Delete();
        if (nodes.size() == 1) // point mesh
        {
            //
            //  Set pick point to be the same as the cell
            //  node for point mesh.
            //
            ds->GetPoint(nodes[0], ppt);
        }
        rv = true;
    }
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryMesh
//
//  Purpose:
//    Queries the db regarding mesh info for a domain. 
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    meshInfo     A place to store the results. 
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     April 18, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Tue Sep  9 16:51:10 PDT 2003
//    Changed PickVarInfo argument to std::string. 
//
// ****************************************************************************

bool
avtGenericDatabase::QueryMesh(const std::string &varName, const int ts,
                              const int dom, std::string &meshInfo)
{
    bool rv = false;
    string mesh = GetMetaData(ts)->MeshForVar(varName);
    const avtMeshMetaData *mmd = GetMetaData(ts)->GetMesh(mesh);
    char temp[256];
    if (!mmd)
    {
         debug5 << "Querying mesh, but could not retrieve"
                << " meta data!" << endl;
         return false;
    }
    sprintf(temp, "%s:  ", mesh.c_str());
    meshInfo += temp;
    if (mmd->numGroups > 0 && dom < mmd->groupIds.size())
    {
         sprintf(temp, " %s %d " , mmd->groupPieceName.c_str(), 
                 mmd->groupIds[dom]);
         meshInfo += temp;
         rv = true;
    }
    if (mmd->numBlocks > 1)
    {
        if ( mmd->blockNames.size() == 0)
        {
             sprintf(temp, " %s %d " , mmd->blockPieceName.c_str(), 
                     dom + mmd->blockOrigin);
             meshInfo += temp;
        }
        else 
        {
             sprintf(temp, " %s %s " , mmd->blockPieceName.c_str(), 
                     mmd->blockNames[dom].c_str());
             meshInfo += temp;
        }
        rv = true;
    }
    // 
    // This is where we could allow the interface to add more information.
    // 
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::QueryZones
//
//  Purpose:
//    Queries the db regarding zones incident upon a specific node. 
//
//  Arguments:
//    varName     The variable on which to retrieve data.
//    dom         The domain to query.
//    foundEl     IN:  zone that contains the picked point. 
//                OUT: The node closest to the picked point.
//    ts          The timestep to query.
//    zones       A place to store the zones.
//    ppt         IN:  The picked point. 
//                OUT: The node coordinates. 
//    dimension   The spatial dimension.
//    useNodeCoords Whether or not to supply coordinates for the node.
//    logical     Whether the node coords should be logical. 
//    nodeCoords  A place to store the node coordinates.
//
//  Returns:
//    True if data was successfully retrieved, false otherwise.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     June 20, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Tue Sep 16 13:33:30 PDT 2003 
//    Use "base_index" if available when creating logical coords. 
//    
// ****************************************************************************

bool
avtGenericDatabase::QueryZones(const string &varName, const int dom, 
                               int &foundEl, const int ts, vector<int> &zones, 
                               float ppt[3], const int dimension,
                               const bool useNodeCoords, const bool logical, 
                               vector<string> &nodeCoords)
{
    string meshName = GetMetaData(ts)->MeshForVar(varName);
    vtkDataSet *ds = GetMeshDataset(meshName.c_str(), ts, dom, "_all");
    bool rv = false; 
    if (ds)
    {
        vtkPoints *points = vtkVisItUtility::GetPoints(ds);
        vtkIdList *ids = vtkIdList::New();
        vtkIdType *idptr; 
        vtkIdType minId = -1;
        vtkIntArray *bi = (vtkIntArray*)ds->GetFieldData()->GetArray("base_index"); 
        int base[3] = {0, 0, 0};
        if (bi)
        {
            base[0] = bi->GetValue(0);
            base[1] = bi->GetValue(1);
            base[2] = bi->GetValue(2);
        }
        if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
        {
           // This method is faster for rectilinear grids than other types 
           // It is also faster for RGrids than method below. 
            minId = ds->FindPoint(ppt);
        }
        else
        {
            ds->GetCellPoints(foundEl, ids);
            int numPts = ids->GetNumberOfIds();
            float dist2;
            float minDist2 = FLT_MAX;
            idptr = ids->GetPointer(0);
            for (int i = 0; i < numPts; i++)
            {
                dist2 = vtkMath::Distance2BetweenPoints(ppt, 
                    points->GetPoint(idptr[i]));
                if (dist2 < minDist2)
                {
                    minDist2 = dist2; 
                    minId = idptr[i]; 
                }
            }
        }

        if ( minId != -1)
        {
            ppt[0] = points->GetPoint(minId)[0];
            ppt[1] = points->GetPoint(minId)[1];
            ppt[2] = points->GetPoint(minId)[2];

            foundEl = minId;

            if (useNodeCoords)
            {
                char buff[80];
                int ijk[3];
                float coord[3];
                int type = ds->GetDataObjectType();
                if (logical && (type == VTK_STRUCTURED_GRID || 
                     type == VTK_RECTILINEAR_GRID))
                {
                    vtkVisItUtility::GetLogicalIndices(ds, false, minId, ijk);
                    ijk[0] += base[0];
                    ijk[1] += base[1];
                    ijk[2] += base[2];
                    if (dimension == 2)
                    {
                        sprintf(buff, "<%d, %d>", ijk[0], ijk[1]);
                    }
                    else 
                    {
                        sprintf(buff, "<%d, %d, %d>", ijk[0], ijk[1], ijk[2]);
                    }
                }
                else
                {
                    points->GetPoint(minId, coord); 
                    if (dimension  == 2)
                    {
                        sprintf(buff, "<%g, %g>", coord[0], coord[1]);
                    }
                    else 
                    {
                        sprintf(buff, "<%g, %g, %g>", coord[0], coord[1], coord[2]);
                    }
                 }
                 nodeCoords.push_back(buff);
            }
            ids->Reset();
            ds->GetPointCells(minId, ids);
            int nCells = ids->GetNumberOfIds();
            if (nCells > 0)
            {
                vtkUnsignedCharArray *ghostArray = (vtkUnsignedCharArray*)
                    ds->GetCellData()-> GetArray("vtkGhostLevels");   
                unsigned char *ghosts = NULL;
                if (ghostArray)
                    ghosts = ghostArray->GetPointer(0);
                idptr = ids->GetPointer(0);
                for (int i = 0;i < nCells; i++)
                {
                    if (ghosts && ghosts[idptr[i]] == 1)
                        continue;
                    zones.push_back(idptr[i]);
                }
                rv = true;
            }
        } // found valid point
        ids->Delete();
    }
    return rv;
}


// ****************************************************************************
//  Method: avtGenericDatabase::AssociateBounds
//
//  Purpose:
//      Calls GetBounds on the dataset one time and associates the information
//      with the dataset.  This avoids having the routine called repeatedly.
//
//  Programmer: Hank Childs
//  Creation:   July 29, 2003
//
// ****************************************************************************

void
avtGenericDatabase::AssociateBounds(vtkDataSet *ds)
{
    float bounds[6];
    ds->GetBounds(bounds);
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetName("avtOriginalBounds");
    arr->SetNumberOfTuples(6);
    for (int i = 0 ; i < 6 ; i++)
        arr->SetTuple1(i, bounds[i]);
    ds->GetFieldData()->AddArray(arr);
    arr->Delete();
}


// ****************************************************************************
//  Method: avtGenericDatabase::ScaleMesh
//
//  Purpose:
//      Scales the mesh in if it is too big.
//
//  Programmer: Hank Childs
//  Creation:   July 29, 2003
//
// ****************************************************************************

void
avtGenericDatabase::ScaleMesh(vtkDataSet *ds)
{
    bool needScaling = false;
    double scaleFactor = 1.;

    float bounds[6];
    if (ds->GetFieldData()->GetArray("avtOriginalBounds") != NULL)
    {
        vtkDataArray *arr = ds->GetFieldData()->GetArray("avtOriginalBounds");
        for (int i = 0 ; i < 6 ; i++)
            bounds[i] = arr->GetTuple1(i);
    }
    else
    {
        ds->GetBounds(bounds);
    }

    if (bounds[1] - bounds[0] > 1e8)
    {
        scaleFactor = 1.;
        double temp = bounds[1] - bounds[0];
        while (temp > 1e6)
        {
            temp /= 10.;
            scaleFactor *= 10.;
        }
        needScaling = true;
    }
    if ((bounds[1] > bounds[0]) && (bounds[1] - bounds[0] < 1e-6))
    {
        scaleFactor = 1.;
        double temp = bounds[1] - bounds[0];
        while (temp < 1e-3)
        {
            temp *= 10.;
            scaleFactor /= 10.;
        }
        needScaling = true;
    }
    if (bounds[3] - bounds[2] > 1e8)
    {
        scaleFactor = 1.;
        double temp = bounds[3] - bounds[2];
        while (temp > 1e6)
        {
            temp /= 10.;
            scaleFactor *= 10.;
        }
        needScaling = true;
    }
    if ((bounds[3] > bounds[2]) && (bounds[3] - bounds[2] < 1e-6))
    {
        scaleFactor = 1.;
        double temp = bounds[3] - bounds[2];
        while (temp < 1e-3)
        {
            temp *= 10.;
            scaleFactor /= 10.;
        }
        needScaling = true;
    }
    if (bounds[5] - bounds[4] > 1e8)
    {
        scaleFactor = 1.;
        double temp = bounds[5] - bounds[4];
        while (temp > 1e6)
        {
            temp /= 10.;
            scaleFactor *= 10.;
        }
        needScaling = true;
    }
    if ((bounds[5] > bounds[4]) && (bounds[5] - bounds[4] < 1e-6))
    {
        scaleFactor = 1.;
        double temp = bounds[5] - bounds[4];
        while (temp < 1e-3)
        {
            temp *= 10.;
            scaleFactor /= 10.;
        }
        needScaling = true;
    }

    if (!needScaling)
    {
        return;
    }

    static bool haveIssuedWarning = false;
    if (!haveIssuedWarning)
    {
        char msg[1024] = "The dataset is too large or too small for VisIt to "
                  "handle natively.  As a result, the dataset is being scaled."
                  "  This will affect the labels for axes, as well as "
                  "coordinate arguments, such as the origin for a slice."
                  "This message will only be issued one time per session, "
                  "even if additional datasets are scaled.";
        avtCallback::IssueWarning(msg);
        haveIssuedWarning = true;
    }

    int dstype = ds->GetDataObjectType();
    switch (dstype)
    {
      case VTK_STRUCTURED_GRID:
      case VTK_UNSTRUCTURED_GRID:
      case VTK_POLY_DATA:
        {
            vtkPointSet *ps = (vtkPointSet *) ds;
            vtkPoints *pts = ps->GetPoints();
            vtkPoints *newPts = vtkPoints::New();
            int npts = pts->GetNumberOfPoints();
            newPts->SetNumberOfPoints(npts);
            float *np = (float *) newPts->GetVoidPointer(0);
            float *p  = (float *) pts->GetVoidPointer(0);
            int nvals = 3*npts;
            for (int i = 0 ; i < nvals ; i++)
            {
                np[i] = p[i] / scaleFactor;
            }
            ps->SetPoints(newPts);
            newPts->Delete();
        }
        break;
      case VTK_RECTILINEAR_GRID:
        {
            vtkRectilinearGrid *rg = (vtkRectilinearGrid *) ds;
            vtkDataArray *in[3];
            in[0] = rg->GetXCoordinates();
            in[1] = rg->GetYCoordinates();
            in[2] = rg->GetZCoordinates();
            vtkFloatArray *out[3];
            for (int i = 0 ; i < 3 ; i++)
            {
                out[i] = vtkFloatArray::New();
                int ntuples = in[i]->GetNumberOfTuples();
                out[i]->SetNumberOfTuples(ntuples);
                for (int j = 0 ; j < ntuples ; j++)
                    out[i]->SetTuple1(j, in[i]->GetTuple1(j) / scaleFactor);
            }
            rg->SetXCoordinates(out[0]);
            rg->SetYCoordinates(out[1]);
            rg->SetZCoordinates(out[2]);
            out[0]->Delete();
            out[1]->Delete();
            out[2]->Delete();
        }
        break;

      default:
        debug1 << "Warning: a mesh should be scaled, but VisIt could not do "
               << "it because it was an unknown meshtype." << endl;
    }

    if (ds->GetFieldData()->GetArray("avtOriginalBounds") != NULL)
    {
        vtkDataArray *arr = ds->GetFieldData()->GetArray("avtOriginalBounds");
        for (int i = 0 ; i < 6 ; i++)
            arr->SetTuple1(i, arr->GetTuple1(i) / scaleFactor);
    }
}


