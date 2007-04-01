// ************************************************************************* //
//                              avtDatabase.C                                //
// ************************************************************************* //

#include <avtDatabase.h>

#include <fstream.h>
#include <stdio.h>
#include <vector>

#include <Expression.h>
#include <PickAttributes.h>
#include <PickVarInfo.h>
#include <Utility.h>

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
#include <ImproperUseException.h>
#include <TimingsManager.h>

// size of MD/SIL caches
int       avtDatabase::mdCacheSize         = 20;
int       avtDatabase::silCacheSize        = 20;

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
    gotIOInfo         = false;
    invariantMetaData = NULL;
    invariantSIL      = NULL;
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
//    Mark C. Miller, 30Sep03 Added support to time-varying SIL/MD
// ****************************************************************************

avtDatabase::~avtDatabase()
{
    std::vector<avtDataObjectSource *>::iterator it;
    for (it = sourcelist.begin() ; it != sourcelist.end() ; it++)
    {
        delete *it;
    }

    if (invariantMetaData != NULL)
    {
        delete invariantMetaData;
        invariantMetaData = NULL;
    }

    if (invariantSIL != NULL)
    {
        delete invariantSIL;
        invariantSIL = NULL;
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
    int nDomains = GetMetaData(ts)->GetNDomains(var);

    if (nDomains <= 0)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    //
    // Create the source object.
    //
    avtDataObjectSource *src = CreateSource(var, ts);
    avtDataObject_p dob = src->GetOutput();

    PopulateDataObjectInformation(dob, var, ts);

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
//    Hank Childs, Tue Sep 23 23:03:07 PDT 2003
//    Add support for tensors.
//
//    Mark C. Miller, 30Sep03
//    Added timeStep argument
//
//    Hank Childs, Mon Feb 23 07:49:51 PST 2004
//    Update for new data attribute interface.  Now add variables for each of
//    the secondary variables in data specification.
//
//    Kathleen Bonnell, Thu Mar 11 11:17:58 PST 2004
//    DataExtents now always has only 2 componnts. 
//
// ****************************************************************************

void
avtDatabase::PopulateDataObjectInformation(avtDataObject_p &dob,
                                           const char *var,
                                           int ts,
                                           avtDataSpecification *spec)
{
    int   i;

    int timerHandle = visitTimer->StartTimer();

    avtDataAttributes &atts     = dob->GetInfo().GetAttributes();
    avtDataValidity   &validity = dob->GetInfo().GetValidity();

    string mesh = GetMetaData(ts)->MeshForVar(var);
    const avtMeshMetaData *mmd = GetMetaData(ts)->GetMesh(mesh);
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
    
    //
    // We want to add information to the data attributes for each of the 
    // variables.  Make a big list of the primary and secondary variables.
    //
    vector<const char *> var_list;
    var_list.push_back(var);
    if (spec != NULL)
    {
        const std::vector<CharStrRef> &secondaryVariables 
                                               = spec->GetSecondaryVariables();
        for (i = 0 ; i < secondaryVariables.size() ; i++)
        {
            var_list.push_back(*(secondaryVariables[i]));
        }
    }

    //
    // Now iterate through our variable list and add information about each
    // variable as we go.
    //
    for (i = 0 ; i < var_list.size() ; i++)
    {
        const avtScalarMetaData *smd = GetMetaData(ts)->GetScalar(var_list[i]);
        if (smd != NULL)
        {
            atts.AddVariable(var_list[i]);
            atts.SetVariableDimension(1, var_list[i]);
            atts.SetCentering(smd->centering, var_list[i]);
    
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
    
                atts.GetTrueDataExtents(var_list[i])->Set(extents);
            }
        }
    
        const avtVectorMetaData *vmd = GetMetaData(ts)->GetVector(var_list[i]);
        if (vmd != NULL)
        {
            atts.AddVariable(var_list[i]);
            atts.SetVariableDimension(vmd->varDim, var_list[i]);
            atts.SetCentering(vmd->centering, var_list[i]);
    
            //
            // Note that we are using the spatial extents as both the spatial 
            // extents and as the global spatial extents (the spatial extents 
            // across all timesteps).
            //
            if (vmd->hasDataExtents)
            {
                double extents [2];
                extents[0] = vmd->minDataExtents;
                extents[1] = vmd->maxDataExtents;
                atts.GetTrueDataExtents(var_list[i])->Set(extents);
            }
        }
    
        const avtTensorMetaData *tmd = GetMetaData(ts)->GetTensor(var_list[i]);
        if (tmd != NULL)
        {
            atts.AddVariable(var_list[i]);
            atts.SetVariableDimension(9, var_list[i]);
            atts.SetCentering(tmd->centering, var_list[i]);
        }
    
        const avtSymmetricTensorMetaData *stmd = 
                                   GetMetaData(ts)->GetSymmTensor(var_list[i]);
        if (stmd != NULL)
        {
            atts.AddVariable(var_list[i]);
            atts.SetVariableDimension(9, var_list[i]);
            atts.SetCentering(stmd->centering, var_list[i]);
        }

        const avtSpeciesMetaData *spmd = 
                                      GetMetaData(ts)->GetSpecies(var_list[i]);
        if (spmd != NULL)
        {
            atts.AddVariable(var_list[i]);
            atts.SetVariableDimension(1, var_list[i]);
            atts.SetCentering(AVT_ZONECENT, var_list[i]);
            double extents[2];
            extents[0] = 0.;
            extents[1] = 1.;
            atts.GetEffectiveDataExtents(var_list[i])->Set(extents);
            atts.GetTrueDataExtents(var_list[i])->Set(extents);
        }

        const avtCurveMetaData *cmd = GetMetaData(ts)->GetCurve(var_list[i]);
        if (cmd != NULL)
        {
            atts.SetTopologicalDimension(1);
            atts.SetSpatialDimension(2);
        }
    }
    atts.SetActiveVariable(var);

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
//  Method: avtDatabase::MetaDataIsInvariant
//
//  Purpose: Return whether or not database meta data is invariant.
//
//  We use a bool* to implement this so that we can know whether or not the
//  bool has been set and once it is set, never call down to the plugins to
//  re-acquire this knowledge
//
//  Programmer: Mark C. Miller, 30Sep03
// ****************************************************************************
bool
avtDatabase::MetaDataIsInvariant(void)
{
   if (invariantMetaData == NULL)
   {
      invariantMetaData = new bool;
      *invariantMetaData = HasInvariantMetaData();
   }

   return *invariantMetaData;
}

// ****************************************************************************
//  Method: avtDatabase::SILIsInvariant
//
//  Purpose: Return whether or not SIL is invariant.
//
//  We use a bool* to implement this so that we can know whether or not the
//  bool has been set and once it is set, never call down to the plugins to
//  re-acquire this knowledge
//
//  Programmer: Mark C. Miller, 30Sep03
// ****************************************************************************
bool
avtDatabase::SILIsInvariant(void)
{
   if (invariantSIL == NULL)
   {
      invariantSIL = new bool;
      *invariantSIL = HasInvariantSIL();
   }

   return *invariantSIL;
}

// ****************************************************************************
//  Method: avtDatabase::GetMostRecentTimestepQueried
//
//  Purpose:
//      Provide convenience method for caller's that don't have a reasonable
//      'current' time to pass to GetMetaData and GetSIL which now both
//      require the caller to specify the time. This function simply returns
//      the time of the most recent request for either SIL or MetaData. The
//      idea is that a reasonable time to use when one is not available is
//      the time of the most recent query for metadata or SIL. 
//      
// ****************************************************************************
int
avtDatabase::GetMostRecentTimestep(void) const
{
   if (sil.size() == 0)
   {
      if (metadata.size() == 0)
         return 0;
      else
         return metadata.front().ts;
   }
   else
   {
      if (metadata.size() == 0)
         return sil.front().ts;
      else
      {
         if (sil.front().ts > metadata.front().ts)
            return sil.front().ts;
         else
            return metadata.front().ts;
      }
   }
}

// ****************************************************************************
//  Method: avtDatabase::GetNewMetaData
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
//    Hank Childs, Thu Aug 14 08:45:39 PDT 2003
//    Set the database's name with the meta-data.
//
//    Mark C. Miller, 22Sep03, changed name to Get'New'MetaData. Put result
//    in MRU cache
//
//    Hank Childs, Tue Nov 25 07:38:18 PST 2003
//    Add mesh quality expressions.
//
//    Hank Childs, Thu Jan 22 10:05:41 PST 2004
//    Do not populate the I/O information if we are only getting meta-data.
//
// ****************************************************************************

void
avtDatabase::GetNewMetaData(int timeState)
{
    // sanity check: since SIL info is currently embedded in MetaData,
    // we cannot have MD invariant but SIL NOT invariant
    if (MetaDataIsInvariant() && !SILIsInvariant())
    {
        EXCEPTION1(ImproperUseException, "invalid MD/SIL invariants condition");
    }

    // acquire new metadata for the given timestep
    avtDatabaseMetaData *md = new avtDatabaseMetaData;
    const char *filename = GetFilename(timeState);
    string fname;
    if (filename == NULL)
        fname = "";
    else
        fname = filename;
    SetDatabaseMetaData(md, timeState);
    md->SetDatabaseName(fname);
    md->SetMustRepopulateOnStateChange(!MetaDataIsInvariant() ||
                                       !SILIsInvariant());

    AddMeshQualityExpressions(md);

    if (! OnlyServeUpMetaData())
    {
        PopulateIOInformation(timeState, ioInfo);
        gotIOInfo = true;
    }

    // put the metadata at the front of the MRU cache
    CachedMDEntry tmp = {md, timeState};
    metadata.push_front(tmp);
}


// ****************************************************************************
//  Method: avtDatabase::AddMeshQualityExpressions
//
//  Purpose:
//      Adds the mesh quality expressions for unstructured and structured
//      meshes.
//
//  Programmer: Hank Childs
//  Creation:   November 25, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Dec 15 14:36:03 PST 2003
//    If there are multiple meshes, put each mesh in its own subdirectory 
//    ('4214).  Also, if the meshes are in subdirectories, set them up so that
//    the expressions code will recognize them ('4195).
//
// ****************************************************************************

void
avtDatabase::AddMeshQualityExpressions(avtDatabaseMetaData *md)
{
    struct MQExprTopoPair
    {
        MQExprTopoPair(const char *s, int t) { mq_expr = s; topo = t; };
        MQExprTopoPair() { ; };
        string mq_expr;
        int    topo;
    };

    int nmeshes = md->GetNumMeshes();
    for (int i = 0 ; i < nmeshes ; i++)
    {
        const avtMeshMetaData *mmd = md->GetMesh(i);
        avtMeshType mt = mmd->meshType;
        if (mt != AVT_CURVILINEAR_MESH && mt != AVT_UNSTRUCTURED_MESH &&
            mt != AVT_SURFACE_MESH)
        {
            continue;
        }

        const int nPairs = 20;
        MQExprTopoPair exprs[nPairs];
        exprs[0]  = MQExprTopoPair("area", 2);
        exprs[1]  = MQExprTopoPair("aspect_gamma", 3);
        exprs[2]  = MQExprTopoPair("aspect", -1);
        exprs[3]  = MQExprTopoPair("condition", -1);
        exprs[4]  = MQExprTopoPair("diagonal", 3);
        exprs[5]  = MQExprTopoPair("dimension", 3);
        exprs[6]  = MQExprTopoPair("jacobian", -1);
        exprs[7]  = MQExprTopoPair("largest_angle", 2);
        exprs[8]  = MQExprTopoPair("oddy", -1);
        exprs[9]  = MQExprTopoPair("relative_size", -1);
        exprs[10] = MQExprTopoPair("scaled_jacobian", -1);
        exprs[11] = MQExprTopoPair("shape", -1);
        exprs[12] = MQExprTopoPair("shape_and_size", -1);
        exprs[13] = MQExprTopoPair("shear", -1);
        exprs[14] = MQExprTopoPair("skew", -1);
        exprs[15] = MQExprTopoPair("smallest_angle", 2);
        exprs[16] = MQExprTopoPair("stretch", -1);
        exprs[17] = MQExprTopoPair("taper", -1);
        exprs[18] = MQExprTopoPair("volume", 3);
        exprs[19] = MQExprTopoPair("warpage", 2);

        int topoDim = mmd->topologicalDimension;
        string name = mmd->name;
        for (int i = 0 ; i < nPairs ; i++)
        {
            if ((topoDim != exprs[i].topo) && (exprs[i].topo != -1))
                continue;

            Expression new_expr;
            char buff[1024];
            if (nmeshes == 1)
                sprintf(buff, "mesh_quality/%s", exprs[i].mq_expr.c_str());
            else
                sprintf(buff, "mesh_quality/%s/%s", name.c_str(),
                                                    exprs[i].mq_expr.c_str());
            new_expr.SetName(buff);
            bool hasSlash = (strstr(name.c_str(), "/") != NULL);
            if (hasSlash)
                sprintf(buff,"%s(<%s>)",exprs[i].mq_expr.c_str(),name.c_str());
            else
                sprintf(buff,"%s(%s)",exprs[i].mq_expr.c_str(),name.c_str());
            new_expr.SetDefinition(buff);
            new_expr.SetType(Expression::ScalarMeshVar);
            md->AddExpression(&new_expr);
        }
    }
}


// ****************************************************************************
//  Method: avtDatabase::GetMetaData
//
//  Purpose:
//      Get and manage metadata from multiple timesteps in an MRU cache 
//
//  Arguments:
//     timeState : The time state that we're interested in.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 30, 2003
//
// ****************************************************************************

avtDatabaseMetaData *
avtDatabase::GetMetaData(int timeState)
{
    if (MetaDataIsInvariant())
    {

        // since its invariant, get it once at specified time 
        if (metadata.size() == 0)
            GetNewMetaData(timeState);

    }
    else
    {
        // see if we've already cached metadata for this timestep
        std::list<CachedMDEntry>::iterator i;
        for (i = metadata.begin(); i != metadata.end(); i++)
        {
            if (timeState == i->ts)
            {
                // move the found entry to front of list
                CachedMDEntry tmp = *i;
                metadata.erase(i);
                metadata.push_front(tmp);
                break;
            }
        }

       // if we didn't find it in the cache, remove the oldest (last) entry
       // and read new metadata
       if (i == metadata.end())
       {
           if (metadata.size() >= mdCacheSize)
           {
               CachedMDEntry tmp = metadata.back(); 
               metadata.pop_back();
               delete tmp.md;
           }

           GetNewMetaData(timeState);
       }

    }

    return metadata.front().md;
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
//    Mark C. Miller, 22Sep03, changed name to Get'New'SIL. Put result
//    in MRU cache
//
// ****************************************************************************

void
avtDatabase::GetNewSIL(int timeState)
{
    // build a new sil for the given timestep
    avtSIL *newsil = new avtSIL;
    PopulateSIL(newsil, timeState);

    // put result in front of MRU cache
    CachedSILEntry tmp = {newsil, timeState};
    sil.push_front(tmp);
}

// ****************************************************************************
//  Method: avtDatabase::GetSIL
//
//  Purpose:
//      Get and manage SILs from multiple timesteps in an MRU cache 
//
//  Arguments:
//     timeState : The time state that we're interested in.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 30, 2003
//
// ****************************************************************************
avtSIL *
avtDatabase::GetSIL(int timeState)
{
    if (SILIsInvariant())
    {

        // since its invariant, get it at time 0
        if (sil.size() == 0)
            GetNewSIL(0);

    }
    else
    {
        // see if we've already cached sil for this timestep
        std::list<CachedSILEntry>::iterator i;
        for (i = sil.begin(); i != sil.end(); i++)
        {
            if (timeState == i->ts)
            {
                // move the found entry to front of list
                CachedSILEntry tmp = *i;
                sil.erase(i);
                sil.push_front(tmp);
                break;
            }
        }

       // if we didn't find it in the cache, remove the oldest (last) entry
       // and read new sil
       if (i == sil.end())
       {
           if (sil.size() >= silCacheSize)
           {
               CachedSILEntry tmp = sil.back(); 
               sil.pop_back();
               delete tmp.sil;
           }

           GetNewSIL(timeState);
       }

    }

    return sil.front().sil;
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
//  Method: avtDatabase::GetIOInformation
//
//  Purpose:
//      Gets the I/O information about the files.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2001
//
//  Modifications:
//    Mark C. Miller, Tue Mar 16 14:49:26 PST 2004
//    Made it call PopulateIOInformation directly 
//
// ****************************************************************************

const avtIOInformation &
avtDatabase::GetIOInformation(int stateIndex)
{
    if (!gotIOInfo)
        PopulateIOInformation(stateIndex, ioInfo);
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
//    Kathleen Bonnell, Tue Sep  9 16:51:10 PDT 2003 
//    Always call QueryMesh, don't keep vars of type AVT_MESH in pickAtts'
//    PickVarInfo.
//   
//    Kathleen Bonnell, Thu Sep 18 07:43:33 PDT 2003 
//    QueryMaterial should use 'real' elements when available. 
//    
//    Hank Childs, Mon Sep 22 09:20:08 PDT 2003
//    Added support for tensors and symmetric tensors.
//
//    Kathleen Bonnell, Fri Oct 24 15:37:41 PDT 2003 
//    Re-add code that makes QueryMaterial use 'real' elements when available. 
//    (was accidentally removed).
//    
//    Kathleen Bonnell, Tue Nov 18 14:07:13 PST 2003 
//    Added support for ZoneCoords. 
//    
//    Kathleen Bonnell, Thu Nov 20 15:17:21 PST 2003 
//    Added support for MATSPECIES vars. 
//    
//    Kathleen Bonnell, Wed Dec 17 14:58:31 PST 2003 
//    Added support for multiple types of coordinates to be returned. 
//
//    Kathleen Bonnell, Mon Mar  8 15:34:13 PST 2004 
//    Allow vars that already have info to be skipped. 
//    
//    Kathleen Bonnell, Tue Mar 16 15:55:18 PST 2004 
//    Don't remove any pickVarInfo's, let Pick handle that.
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
    std::vector<std::string> pnodeCoords  = pa->GetPnodeCoords();
    std::vector<std::string> dnodeCoords  = pa->GetDnodeCoords();
    std::vector<std::string> bnodeCoords  = pa->GetBnodeCoords();
    std::vector<std::string> dzoneCoords  = pa->GetDzoneCoords();
    std::vector<std::string> bzoneCoords  = pa->GetBzoneCoords();
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

        bool logicalDNodes = pa->GetShowNodeDomainLogicalCoords();
        bool logicalBNodes = pa->GetShowNodeBlockLogicalCoords();
        bool physicalNodes = pa->GetShowNodePhysicalCoords();
        bool logicalDZones = pa->GetShowZoneDomainLogicalCoords(); 
        bool logicalBZones = pa->GetShowZoneBlockLogicalCoords();

        if (zonePick)
        {
            success = QueryNodes(vName, foundDomain, foundEl, ts, incEls, ppt, 
                          pa->GetDimension(), physicalNodes, logicalDNodes,
                          logicalBNodes, pnodeCoords, dnodeCoords, bnodeCoords,
                          logicalDZones, logicalBZones, dzoneCoords, bzoneCoords);
        }
        else       
        {
            success = QueryZones(vName, foundDomain, foundEl, ts, incEls, cpt,
                          pa->GetDimension(), physicalNodes, logicalDNodes,
                          logicalBNodes, pnodeCoords, dnodeCoords, bnodeCoords,
                          logicalDZones, logicalBZones, dzoneCoords, bzoneCoords);
            if (success)
                pa->SetElementNumber(foundEl);
        }
        if (success)
        {
            pa->SetFulfilled(true);
            pa->SetIncidentElements(incEls);
            pa->SetPnodeCoords(pnodeCoords);
            pa->SetDnodeCoords(dnodeCoords);
            pa->SetBnodeCoords(bnodeCoords);
            pa->SetDzoneCoords(dzoneCoords);
            pa->SetBzoneCoords(bzoneCoords);
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

    std::string meshInfo;
    QueryMesh(pa->GetActiveVariable(), ts, foundDomain, meshInfo);
    pa->SetMeshInfo(meshInfo);

    for (int varNum = 0; varNum < userVars.size(); varNum++)
    {
        vName = userVars[varNum];
        if (strcmp(vName.c_str(), "default") == 0)
        {
            vName = pa->GetActiveVariable();
        }
        // 
        // Skip any variables that already have info, unless they are scalar,
        // because then MatFracs might be necessary.
        // 
        if (pa->GetPickVarInfo(varNum).HasInfo() &&
            strcmp(pa->GetPickVarInfo(varNum).GetVariableType().c_str(), "scalar") != 0)
        {
            continue;
        }
        bool success = false;
        TRY
        {
            avtVarType varType = GetMetaData(ts)->DetermineVarType(vName);
            int matEl = (pa->GetRealElementNumber() != -1 ? 
                         pa->GetRealElementNumber() : foundEl);
            intVector matIncEls = (pa->GetRealIncidentElements().size() > 0 ? 
                                   pa->GetRealIncidentElements() : incEls);
            switch(varType)
            {
                case AVT_SCALAR_VAR : success = 
                   QueryScalars(vName, foundDomain, foundEl, ts, incEls, 
                                pa->GetPickVarInfo(varNum), zonePick);
                   pa->GetPickVarInfo(varNum).SetVariableType("scalar");
                   break; 
                case AVT_VECTOR_VAR : success = 
                   QueryVectors(vName, foundDomain, foundEl, ts, incEls, 
                                pa->GetPickVarInfo(varNum), zonePick);
                   pa->GetPickVarInfo(varNum).SetVariableType("vector");
                   break; 
                case AVT_TENSOR_VAR : success = 
                   QueryTensors(vName, foundDomain, foundEl, ts, incEls, 
                                pa->GetPickVarInfo(varNum), zonePick);
                   pa->GetPickVarInfo(varNum).SetVariableType("tensor");
                   break; 
                case AVT_SYMMETRIC_TENSOR_VAR : success = 
                   QuerySymmetricTensors(vName, foundDomain, foundEl, ts,
                                 incEls, pa->GetPickVarInfo(varNum), zonePick);
                   pa->GetPickVarInfo(varNum).SetVariableType("symm_tensor");
                   break; 
                case AVT_MATERIAL : success = 
                   QueryMaterial(vName, foundDomain, matEl, ts, matIncEls, 
                                 pa->GetPickVarInfo(varNum), zonePick);
                   pa->GetPickVarInfo(varNum).SetVariableType("material");
                   break; 
                case AVT_MATSPECIES : success = 
                   QuerySpecies(vName, foundDomain, matEl, ts, matIncEls, 
                                pa->GetPickVarInfo(varNum), zonePick);
                   pa->GetPickVarInfo(varNum).SetVariableType("species");
                   break; 
                case AVT_MESH : 
                   if (!pa->GetPickVarInfo(varNum).HasInfo())
                       pa->GetPickVarInfo(varNum).SetVariableType("mesh");
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

