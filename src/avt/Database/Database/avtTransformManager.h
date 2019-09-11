// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtTransformManager.h                           //
// ************************************************************************* //

#ifndef AVT_TRANSFORM_MANAGER_H
#define AVT_TRANSFORM_MANAGER_H

#include <database_exports.h>

#include <string>
#include <vector>

#include <avtDataRequest.h>
#include <avtMaterial.h>
#include <avtVariableCache.h>

struct avtDatasetCollection;
class avtDatabaseMetaData;
class avtSourceFromDatabase;

#include <vectortypes.h>

// ****************************************************************************
//  Class: avtTransformManager
//
//  Purpose: Manage data transformation services. Native data as read from the
//  plugin is cached in Generic DB's cache. Transformed data is cached here. 
//  
//  The transform manager's primary function is to replace items in the
//  dataset collection in their native form (as read or cached in Generic DB)
//  with their transformed counterparts (as computed or cached here). Because
//  not all data returned to VisIt from Generic DB comes through the
//  the dataset collection, there are some specialized services.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 9, 2006 
//
//  Modifications:
//
//    Hank Childs, Fri May  9 15:59:10 PDT 2008
//    Change signature of certain methods to allow passing of domain IDs.
//    This is needed for performance reasons when there are a large number
//    of entries in the cache.
//
//    Hank Childs, Mon Aug 25 16:16:57 PDT 2008
//    Add a function to clear the cache.
//
//    Mark C. Miller, Thu Feb 12 11:38:37 PST 2009
//    Added method to convert 1d scalar vars to curves
//
//    Mark C. Miller, Tue Feb 17 17:57:54 PST 2009
//    Added FindMatchingCSGDiscretization to handle CSG meshes that don't
//    vary with time.
//
//    Mark C. Miller, Wed May  6 13:51:05 PDT 2009
//    Pass md and dom to AddVertexCellsToPointsOnlyDataset.
//
//    Mark C. Miller, Mon Nov  9 10:40:34 PST 2009
//    Changed interface to main transform method to operate on a single
//    dataset instead of a dataset collection.
//
//    Brad Whitlock, Sun Apr 22 00:01:35 PDT 2012
//    I added some methods that test for excess precision.
//
//    Eric Brugger, Wed Jul 25 09:01:14 PDT 2012
//    I modified the multi-pass discretizion of CSG meshes to only process
//    a portion of the mesh on each processor instead of the entire mesh.
//
//    Kathleen Biagas, Wed Aug  7 15:42:57 PDT 2013
//    Add methods that test for insufficient precision.
//
//    Eric Brugger, Wed Nov 19 08:45:34 PST 2014
//    I reduced the number of reads of CSG meshes to only once per CSG mesh
//    instead of once per region in order to reduce the number of times the
//    same CSG mesh was cached. Typically there is one CSG mesh with many
//    regions, so this is a significant saving. CSG meshes with thousands
//    of regions were exhausting memory in the previous scheme.
//
//    Kathleen Biagas, Mon Dec 22 10:10:29 PST 2014
//    Added RemoveDuplicateNodes.
//
// ****************************************************************************

class DATABASE_API avtTransformManager
{
  public:
                               avtTransformManager(avtVariableCache *gdbCache);
                              ~avtTransformManager();

    void                       FreeUpResources(int lastts);

    vtkDataSet                *TransformSingleDataset(vtkDataSet *ds,
                                   int domain, avtDataRequest_p &spec,
                                   avtSourceFromDatabase *src,
                                   boolVector &selectionsApplied,
                                   avtDatabaseMetaData *md);

    bool                       TransformMaterialDataset(avtDatabaseMetaData *md,
                                   const avtDataRequest_p &spec, avtMaterial **mat, int);

    void                       ClearTimestep(int ts) { cache.ClearTimestep(ts); };

  private:
    bool                       CoordinatesHaveExcessPrecision(vtkDataSet *ds,
                                   bool needNativePrecision) const;
    bool                       DataHasExcessPrecision(vtkDataArray *da, 
                                   bool needNativePrecision) const;
    bool                       CoordinatesHaveInsufficientPrecision(
                                   vtkDataSet *ds,
                                   bool needNativePrecision) const;
    bool                       DataHasInsufficientPrecision(vtkDataArray *da,
                                   bool needNativePrecision) const;

    vtkDataSet                *NativeToFloat(const avtDatabaseMetaData *const md,
                                             const avtDataRequest_p &spec,
                                             vtkDataSet *ds, int dom);
    vtkDataSet                *FindMatchingCSGDiscretization(
                                   const avtDatabaseMetaData *const md,
                                   const avtDataRequest_p &dataRequest,
                                   const char *vname, const char *type,
                                   int ts, int csgdom, int dom,
                                   const char *mat);
    vtkDataSet                *CSGToDiscrete(avtDatabaseMetaData *md,
                                             const avtDataRequest_p &spec,
                                             vtkDataSet *ds, int);
    vtkDataSet                *AddVertexCellsToPointsOnlyDataset(avtDatabaseMetaData *md,
                                   vtkDataSet *ds, int dom);
    vtkDataSet                *ConvertCurvesToRectGrids(avtDatabaseMetaData *md,
                                   vtkDataSet *ds, int dom);

    vtkDataSet                *RemoveDuplicateNodes(vtkDataSet *ds);

    vtkDataSet                *OverflowInducingToEmpty(avtDatabaseMetaData *md,
                                   vtkDataSet *ds, int dom);

    avtVariableCache           cache;
    avtVariableCache          *gdbCache;
};

#endif
