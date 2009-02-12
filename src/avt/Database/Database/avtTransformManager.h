/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
// ****************************************************************************

class DATABASE_API avtTransformManager
{
  public:
                               avtTransformManager(avtVariableCache *gdbCache);
                              ~avtTransformManager();

    void                       FreeUpResources(int lastts);

    bool                       TransformDataset(avtDatasetCollection &ds,
                                   intVector &domains, avtDataRequest_p &spec,
                                   avtSourceFromDatabase *src,
                                   boolVector &selectionsApplied,
                                   avtDatabaseMetaData *md);

    bool                       TransformMaterialDataset(const avtDatabaseMetaData *const md,
                                   const avtDataRequest_p &spec, avtMaterial **mat, int);

    void                       ClearTimestep(int ts) { cache.ClearTimestep(ts); };

  private:
    vtkDataSet                *NativeToFloat(const avtDatabaseMetaData *const md,
                                             const avtDataRequest_p &spec,
                                             vtkDataSet *ds, int dom);
    vtkDataSet                *CSGToDiscrete(const avtDatabaseMetaData *const md,
                                             const avtDataRequest_p &spec,
                                             vtkDataSet *ds, int);
    vtkDataSet                *AddVertexCellsToPointsOnlyDataset(vtkDataSet *ds);
    vtkDataSet                *ConvertCurvesToRectGrids(avtDatabaseMetaData *md,
                                   vtkDataSet *ds, int dom);

    avtVariableCache           cache;
    avtVariableCache          *gdbCache;
};

#endif
