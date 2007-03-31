// ************************************************************************* //
//                              avtDatabase.C                                //
// ************************************************************************* //

#include <avtDatabase.h>

#include <fstream.h>
#include <stdio.h>
#include <vector>

#include <avtDatabaseMetaData.h>
#include <avtDataObjectSource.h>
#include <avtDataset.h>
#include <avtExtents.h>
#include <avtIOInformation.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtSIL.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <PickAttributes.h>
#include <PickVarInfo.h>
#include <TimingsManager.h>
#include <Utility.h>

bool      avtDatabase::onlyServeUpMetaData = false;


// ****************************************************************************
//  Method: avtDatabase constructor
//
//  Arguments:
//      f       The list of files.
//      nf      The number of files in f.
//
//  Programmer: Hank Childs
//  Creation:   August 9, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Aug 14 11:49:24 PDT 2000
//    Made a database be more tied to its files by putting them in constructor.
//
//    Hank Childs, Wed Sep 13 20:56:28 PDT 2000
//    Moved initialization of files to base class.
//
//    Hank Childs, Thu Mar  1 13:42:43 PST 2001
//    Split class so functionality went in derived type, avtGenericDatabase.
//
//    Hank Childs, Fri Aug 31 17:01:18 PDT 2001
//    Initialized gotIOInfo.
//
// ****************************************************************************

avtDatabase::avtDatabase()
{
    metadata        = NULL;
    sil             = NULL;
    gotIOInfo       = false;
}


// ****************************************************************************
//  Method: avtDatabase destructor
//
//  Programmer: Hank Childs
//  Creation:   August 9, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Aug 14 09:28:41 PDT 2000
//    Added deletion of variable and source as arrays.
//
//    Hank Childs, Wed Sep 13 20:56:28 PDT 2000
//    Moved deletion of files down to base class.
//
//    Hank Childs, Thu Mar  1 13:42:43 PST 2001
//    Split class so functionality went in derived type, avtGenericDatabase.
//
// ****************************************************************************

avtDatabase::~avtDatabase()
{
    if (metadata != NULL)
    {
        delete metadata;
        metadata = NULL;
    }
    
    std::vector<avtDataObjectSource *>::iterator it;
    for (it = sourcelist.begin() ; it != sourcelist.end() ; it++)
    {
        delete *it;
    }

    if (sil != NULL)
    {
        delete sil;
        sil = NULL;
    }
}


// ****************************************************************************
//  Method: avtDatabase::GetOutput
//
//  Purpose:
//      Gets an avtDataset object that corresponds to the currently opened
//      files and specified variable.
//
//  Arguments:
//      var     The variable that the dataset should correspond to.
//      ts      The timestep that the dataset should correspond to.
//
//  Returns:    An avtDataset corresponding to the files and variable.
//
//  Programmer: Hank Childs
//  Creation:   August 9, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Aug 14 09:42:50 PDT 2000
//    Re-wrote to handle multiple variables.
//
//    Hank Childs, Thu Mar  1 09:01:30 PST 2001
//    Added timestep argument.  Rewrote to not need data members.
//
//    Hank Childs, Fri Aug 17 16:03:44 PDT 2001
//    Removed dependencies on avtDataset.
//
// ****************************************************************************

avtDataObject_p
avtDatabase::GetOutput(const char *var, int ts)
{
    //
    // Figure out how many domains there are for the current variable.
    //
    int nDomains = GetMetaData()->GetNDomains(var);

    if (nDomains <= 0)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    //
    // Create the source object.
    //
    avtDataObjectSource *src = CreateSource(var, ts);
    avtDataObject_p dob = src->GetOutput();

    PopulateDataObjectInformation(dob, var);

    sourcelist.push_back(src);

    return dob;
}


// ****************************************************************************
//  Method: avtDatabase::PopulateDataObjectInformation
//
//  Purpose:
//      Populates the information from a data object from the database 
//      metadata.
//
//  Arguments:
//      dob       The data object to populate.
//      var       The variable the data object corresponds to.
//
//  Programmer:   Hank Childs
//  Creation:     October 26, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Dec 22 14:10:22 PST 2000
//    Accounted for interface change with dataset information.
//
//    Hank Childs, Mon Mar 19 16:22:25 PST 2001
//    Accounted for centering, vector vars.
//
//    Hank Childs, Sun Mar 25 11:55:28 PST 2001
//    Accounted for new data object information interface.
//
//    Hank Childs, Fri Aug 17 16:07:26 PDT 2001
//    Renamed to get away from avtDataset roots.
//
//    Eric Brugger, Mon Nov  5 13:28:45 PST 2001
//    Modified to always compile the timing code.
//
//    Hank Childs, Wed Dec 19 20:18:19 PST 2001
//    Added species support.
//
//    Hank Childs, Fri Dec 21 07:23:47 PST 2001
//    Write out variable names.
//
//    Hank Childs, Sun Jun 16 19:52:06 PDT 2002
//    Pass on the cell and block origins.
//
//    Hank Childs, Sun Aug 18 11:09:55 PDT 2002
//    Pass on disjoint elements information.
//
//    Hank Childs, Fri Sep 27 16:45:38 PDT 2002
//    Pass on units.
//
//    Hank Childs, Mon Sep 30 09:06:44 PDT 2002
//    Pass on containsGhostZones.
//
//    Hank Childs, Tue Oct  1 14:59:20 PDT 2002
//    If we are doing a material plot, then the output's zones will be
//    invalidated.
//
//    Kathleen Bonnell, Wed Mar 26 13:03:54 PST 2003 
//    Pass on containsOriginalCells.
//
//    Jeremy Meredith, Thu Jun 12 08:48:20 PDT 2003
//    Added logic to decrement the topo. dimension if we have requested
//    material boundaries.
//
//    Hank Childs, Fri Aug  1 21:54:08 PDT 2003
//    Add support for curves.
//
//    Hank Childs, Wed Aug 13 11:17:58 PDT 2003
//    No longer assume that requesting a material means that we are doing
//    material interface reconstruction.
//
// ****************************************************************************

void
avtDatabase::PopulateDataObjectInformation(avtDataObject_p &dob,
                                           const char *var,
                                           avtDataSpecification *spec)
{
    int timerHandle = visitTimer->StartTimer();

    avtDataAttributes &atts     = dob->GetInfo().GetAttributes();
    avtDataValidity   &validity = dob->GetInfo().GetValidity();

    string mesh = GetMetaData()->MeshForVar(var);
    const avtMeshMetaData *mmd = GetMetaData()->GetMesh(mesh);
    if (mmd != NULL)
    {
        atts.SetCellOrigin(mmd->cellOrigin);
        atts.SetBlockOrigin(mmd->blockOrigin);
        atts.SetTopologicalDimension(mmd->topologicalDimension);
        atts.SetSpatialDimension(mmd->spatialDimension);
        atts.SetXUnits(mmd->xUnits);
        atts.SetYUnits(mmd->yUnits);
        atts.SetZUnits(mmd->zUnits);
        atts.SetContainsGhostZones(mmd->containsGhostZones);
        atts.SetContainsOriginalCells(mmd->containsOriginalCells);
        validity.SetDisjointElements(mmd->disjointElements);

        //
        // Note that we are using the spatial extents as both the spatial 
        // extents and as the global spatial extents (the spatial extents 
        // across all timesteps).
        //
        if (mmd->hasSpatialExtents)
        {
            double extents[6];
            for (int i = 0 ; i < mmd->spatialDimension ; i++)
            {
                extents[2*i]   = mmd->minSpatialExtents[i];
                extents[2*i+1] = mmd->maxSpatialExtents[i];
            }
            atts.GetTrueSpatialExtents()->Set(extents);
        }
    }
    
    const avtScalarMetaData *smd = GetMetaData()->GetScalar(var);
    if (smd != NULL)
    {
        atts.SetVariableDimension(1);
        atts.SetVariableName(var);
        atts.SetCentering(smd->centering);

        //
        // Note that we are using the spatial extents as both the spatial 
        // extents and as the global spatial extents (the spatial extents 
        // across all timesteps).
        //
        if (smd->hasDataExtents)
        {
            double extents[2];
            extents[0] = smd->minDataExtents;
            extents[1] = smd->maxDataExtents;

            atts.GetTrueDataExtents()->Set(extents);
        }
    }

    const avtVectorMetaData *vmd = GetMetaData()->GetVector(var);
    if (vmd != NULL)
    {
        atts.SetVariableDimension(vmd->varDim);
        atts.SetVariableName(var);
        atts.SetCentering(vmd->centering);

        //
        // Note that we are using the spatial extents as both the spatial 
        // extents and as the global spatial extents (the spatial extents 
        // across all timesteps).
        //
        if (vmd->hasDataExtents)
        {
            double *extents = new double[2*vmd->varDim];
            for (int i = 0 ; i < vmd->varDim ; i++)
            {
                extents[2*i  ] = vmd->minDataExtents[i];
                extents[2*i+1] = vmd->maxDataExtents[i];
            }

            atts.GetTrueDataExtents()->Set(extents);

            delete [] extents;
        }
    }

    const avtSpeciesMetaData *spmd = GetMetaData()->GetSpecies(var);
    if (spmd != NULL)
    {
        atts.SetVariableDimension(1);
        atts.SetCentering(AVT_ZONECENT);
        atts.SetVariableName(var);
        double extents[2];
        extents[0] = 0.;
        extents[1] = 1.;
        atts.GetEffectiveDataExtents()->Set(extents);
    }

    const avtCurveMetaData *cmd = GetMetaData()->GetCurve(var);
    if (cmd != NULL)
    {
        atts.SetTopologicalDimension(1);
        atts.SetSpatialDimension(2);
    }

    //
    // SPECIAL CASE:
    //
    // We need to decrement the topological dimension if we asked for
    // an unfilled boundary.  The way this is handles (by directly
    // checking a data specification) needs to change.
    //
    if (spec && spec->NeedBoundarySurfaces())
        atts.SetTopologicalDimension(atts.GetTopologicalDimension() - 1);

    char str[1024];
    sprintf(str, "Populating Information for %s", var);
    visitTimer->StopTimer(timerHandle, str);
    visitTimer->DumpTimings();
}


// ****************************************************************************
//  Method: avtDatabase::GetMetaData
//
//  Purpose:
//      Provide mechanism for clients to get information about the file
//      and the variables in the file.
//
//  Arguments:
//     timeState : The time state that we're interested in.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 22, 2000
//
//  Modifications:
//    Hank Childs, Fri May 11 14:10:59 PDT 2001
//    Also populate the IO info when you get the meta-data.
//
//    Brad Whitlock, Wed May 14 09:11:03 PDT 2003
//    Added timeState argument.
//
// ****************************************************************************

avtDatabaseMetaData *
avtDatabase::GetMetaData(int timeState)
{
    if (metadata == NULL)
    {
        metadata = new avtDatabaseMetaData;
        SetDatabaseMetaData(metadata, timeState);
        PopulateIOInformation(ioInfo);
        gotIOInfo = true;
    }

    return metadata;
}


// ****************************************************************************
//  Method: avtDatabase::GetSIL
// 
//  Purpose:
//     Gets the SIL for this database.
//
//  Arguments:
//     timeState : The time state that we're interested in.
//
//  Returns:    The SIL object.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
//  Modifications:
//    Brad Whitlock, Wed May 14 09:10:23 PDT 2003
//    Added timeState argument.
//
// ****************************************************************************

avtSIL *
avtDatabase::GetSIL(int timeState)
{
    if (sil == NULL)
    {
        sil = new avtSIL;
        PopulateSIL(sil, timeState);
    }

    return sil;
}


// ****************************************************************************
//  Method: avtDatabase::ClearCache
//
//  Purpose:
//      Allows the derived types to clear their caches.  This method does
//      nothing and should be re-defined for derived types where clearing the
//      cache is meaningful.
//
//  Programmer: Hank Childs
//  Creation:   May 1, 2001
//
// ****************************************************************************

void
avtDatabase::ClearCache(void)
{
    ;
}

// ****************************************************************************
//  Method: avtDatabase::FreeUpResources
//
//  Purpose:
//      Allows the derived types to free system resources.  This method does
//      nothing and should be re-defined for derived types where freeing
//      system resources is meaningful.
//
//  Programmer: Sean Ahern
//  Creation:   Tue May 21 15:47:28 PDT 2002
//
// ****************************************************************************
void
avtDatabase::FreeUpResources(void)
{
    ;
}


// ****************************************************************************
//  Method: avtDatabase::PopulateIOInformation
//
//  Purpose:
//      This is defined so the derived types don't have to.  It should
//      populate the "I/O Hints", which allows domains that should be processed
//      on the same processor to be groups together when the load balancing
//      occurs.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2001
//
// ****************************************************************************

void
avtDatabase::PopulateIOInformation(avtIOInformation &)
{
    ;
}


// ****************************************************************************
//  Method: avtDatabase::GetIOInformation
//
//  Purpose:
//      Gets the I/O information about the files.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2001
//
// ****************************************************************************

const avtIOInformation &
avtDatabase::GetIOInformation(void)
{
    if (!gotIOInfo)
    {
        //
        // Getting the meta-data will force the I/O information to be read in.
        // It also does the correct preconditions so that getting the I/O info
        // will be meaningful.
        //
        GetMetaData();
    }
    return ioInfo;
}


// ****************************************************************************
//  Method: avtDatabase::CanDoDynamicLoadBalancing
//
//  Purpose:
//      Determines whether or not we can do dynamic load balancing.
//
//  Programmer: Hank Childs
//  Creation:   October 25, 2001
//
// ****************************************************************************

bool
avtDatabase::CanDoDynamicLoadBalancing(void)
{
    return true;
}


// ****************************************************************************
//  Method: avtDatabase::NumStagesForFetch
//
//  Purpose:
//      This returns how many stages there are for the fetch.  Some databases
//      have more than one stage (read from file format, perform material
//      selection, etc).
//
//  Returns:    A good default number of stages (1).
//
//  Programmer: Hank Childs
//  Creation:   October 26, 2001
//
// ****************************************************************************

int
avtDatabase::NumStagesForFetch(avtDataSpecification_p)
{
    return 1;
}


// ****************************************************************************
//  Function: GetFileListFromTextFile
//
//  Purpose:
//      Gets a file list from inside a text file.
//
//  Arguments:
//      textfile    The name of the text file.
//      filelist    A place to put the file list.
//      filelistN   The number of files placed in filelist.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Feb 15 08:42:29 PST 2002
//    Allow for '#' entries.
//
//    Jeremy Meredith, Tue Aug 27 11:34:35 PDT 2002
//    Moved from avtDatabaseFactory to avtDatabase.
//
// ****************************************************************************

void
avtDatabase::GetFileListFromTextFile(const char *textfile,
                                     char **&filelist, int &filelistN)
{
    ifstream ifile(textfile);

    if (ifile.fail())
    {
        EXCEPTION1(InvalidFilesException, textfile);
    }

    char          dir[1024];
    const char   *p = textfile, *q = NULL;
    while ((q = strstr(p, "/")) != NULL)
    {
        p = q+1;
    }
    strncpy(dir, textfile, p-textfile);
    dir[p-textfile] = '\0';

    std::vector<char *>  list;
    char  str_auto[1024];
    char  str_with_dir[1024];
    int   count = 0;
    while (!ifile.eof())
    {
        str_auto[0] = '\0';
        ifile.getline(str_auto, 1024, '\n');
        if (str_auto[0] != '\0' && str_auto[0] != '#')
        {
            if (str_auto[0] == '/' || str_auto[0] == '!')
            {
                strcpy(str_with_dir, str_auto);
            }
            else
            {
                sprintf(str_with_dir, "%s%s", dir, str_auto);
            }
            char *str_heap = CXX_strdup(str_with_dir);
            list.push_back(str_heap); 
            count++;
        }
    }

    filelist = new char*[count];
    std::vector<char *>::iterator it;
    filelistN = 0;
    for (it = list.begin() ; it != list.end() ; it++)
    {
        filelist[filelistN++] = *it;
    }
}


// ****************************************************************************
//  Method: avtDatabase::Query
//
//  Purpose:
//    Queries the db regarding var info for a specific pick point.
//
//  Arguments:
//    dspec   A database specification.
//    pa      The pick attributes in which to story the var information. 
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     November 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec  6 16:25:20 PST 2002
//    Fill in node information if missing.
//    
//    Kathleen Bonnell, Fri Dec 27 14:09:40 PST 2002 
//    Added nodeCoords argument to QueryNodes, in case user requested to
//    know the coordinates of nodes. 
//    
//    Kathleen Bonnell, Fri Apr 18 14:11:24 PDT 2003   
//    Added call to QueryMesh.
//    
//    Hank Childs, Thu May  8 09:05:12 PDT 2003
//    Include the correct file name for the query.
//
//    Kathleen Bonnell, Fri Jun 27 16:54:31 PDT 2003  
//    Support NodePicking -- added call to QueryScalars.  Pass to Query
//    methods whether or not this is a zone pick.  Reflect some name changes
//    in pickAtts.
//
// ****************************************************************************

void               
avtDatabase::Query(PickAttributes *pa)
{
    int ts          = pa->GetTimeStep();
    int foundDomain = pa->GetDomain();
    int foundEl     = pa->GetElementNumber();
    int zonePick    = pa->GetPickType() == PickAttributes::Zone;
    float *PPT, *CPT, ppt[3], cpt[3];
    std::vector<int> incEls  = pa->GetIncidentElements();
    std::vector<std::string> nodeCoords  = pa->GetNodeCoords();
    vector<string> userVars = pa->GetVariables();
    std::string vName; 

    //
    //  Filling the incidentElements is usually done by PickQuery,
    //  but if matSelect has been applied, then the values from PickQuery
    //  won't make sense.  Instead, retrieve the incidentElements here.
    //
    if (incEls.empty())
    {
        PPT = pa->GetPickPoint();
        ppt[0] = PPT[0];
        ppt[1] = PPT[1];
        ppt[2] = PPT[2];
        CPT = pa->GetCellPoint();
        cpt[0] = CPT[0];
        cpt[1] = CPT[1];
        cpt[2] = CPT[2];
        vName = pa->GetActiveVariable();
        bool success; 

        if (zonePick)
        {
            success = QueryNodes(vName, foundDomain, foundEl, ts, incEls, ppt, 
                          pa->GetDimension(), pa->GetUseNodeCoords(), 
                          pa->GetLogicalCoords(), nodeCoords);
        }
        else       
        {
            success = QueryZones(vName, foundDomain, foundEl, ts, incEls, cpt,
                          pa->GetDimension(), pa->GetUseNodeCoords(), 
                          pa->GetLogicalCoords(), nodeCoords);
            if (success)
                pa->SetElementNumber(foundEl);
        }
        if (success)
        {
            pa->SetFulfilled(true);
            pa->SetIncidentElements(incEls);
            pa->SetNodeCoords(nodeCoords);
            pa->SetPickPoint(ppt);
            pa->SetCellPoint(cpt);
        }
        else
        {
            pa->SetFulfilled(false);
            return;
        }
        for (int j = 0; j < userVars.size(); j++)
        {
            PickVarInfo varInfo;
            varInfo.SetVariableName(userVars[j]);
            pa->AddPickVarInfo(varInfo); 
        }
    }


    for (int varNum = 0; varNum < userVars.size(); varNum++)
    {
        vName = userVars[varNum];
        if (strcmp(vName.c_str(), "default") == 0)
        {
            vName = pa->GetActiveVariable();
        }
        bool success = false;
        TRY
        {
            avtVarType varType = metadata->DetermineVarType(vName);
            switch(varType)
            {
                case AVT_SCALAR_VAR : success = 
                   QueryScalars(vName, foundDomain, foundEl, ts, incEls, 
                                pa->GetPickVarInfo(varNum), zonePick);
                   break; 
                case AVT_VECTOR_VAR : success = 
                   QueryVectors(vName, foundDomain, foundEl, ts, incEls, 
                                pa->GetPickVarInfo(varNum), zonePick);
                   break; 
                case AVT_MATERIAL : success = 
                   QueryMaterial(vName, foundDomain, foundEl, ts, incEls, 
                                 pa->GetPickVarInfo(varNum), zonePick);
                   break; 
                case AVT_MESH : success = 
                   QueryMesh(vName, foundDomain, pa->GetPickVarInfo(varNum));
                   break; 
                default : 
                   break; 
            }
        }
        CATCH (InvalidVariableException)
        {
            // User entered a bad variable, but we want to continue processing
            // the other variables, so do nothing with the exception. 
            // "No Information Found" will be listed next to the var in the 
            // pick window.
        }
        ENDTRY 
    }

    const char *fname = GetFilename(ts);
    pa->SetDatabaseName(fname);
}


