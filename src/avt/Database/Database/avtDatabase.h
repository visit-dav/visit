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
//                                 avtDatabase.h                             //
// ************************************************************************* //

#ifndef AVT_DATABASE_H
#define AVT_DATABASE_H

#include <database_exports.h>

#include <vector>
#include <list>

#include <void_ref_ptr.h>

#include <avtDataRequest.h>
#include <avtDataset.h>
#include <avtIOInformation.h>
#include <avtTypes.h>
#include <vectortypes.h>


class   vtkDataSet;

class   avtDatabaseMetaData;
class   avtDataValidity;
class   avtDataObjectSource;
class   avtSIL;
class   PickAttributes;
class   PickVarInfo;


// structures to support SIL/MD MRU caches
typedef struct {
   avtSIL *sil;
   int     ts;
} CachedSILEntry;

typedef struct {
   avtDatabaseMetaData *md;
   int                  ts;
} CachedMDEntry;

// ****************************************************************************
//  Class: avtDatabase
//
//  Purpose:
//      Provides an interface for what our database looks like.  Derived types
//      should have no public functions besides constructors and destructors.
//
//      Regarding invariance of MetaData and/or SIL. The avtDatabaseMetaData
//      object is a catch-all for a lot of bits and pieces of information
//      about a database. In fact, it also currently contains the information
//      necessary to construct a SIL (either a generic one or a custom one).
//      While it would be best to flag each group of related constructs in
//      avtDatabaseMetaData as either invariant or not, we have only culled
//      out the SIL. Nonetheless, there are often cases where only portions
//      of avtDatabaseMetaData vary with time while other portions of it can
//      be assumed to be constant. Examples are the number and names of
//      materials in the problem or the type of set a particular subset in
//      the SIL is. In these cases, it is acceptable to simply request any
//      state's metadata and so state zero is customarily used.
//
//      If there are situations in which you need to obtain MetaData or
//      SIL information but do not have a handy 'current' time, we have 
//      provided a convenience funciton, GetMostRecentTimestep() to return
//      the most recent timestep at which MD/SIL was last queried.
//
//  Programmer: Hank Childs
//  Creation:   August 9, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Mar  7 13:16:05 PST 2001
//    Re-wrote database so that information about file formats could be pushed
//    into its own class.  Blew away previous, now inapplicable comments.  Also
//    changed interface to deal with SIL restrictions.  Further changed
//    interface to return auxiliary data through an arbitrary mechanism.
//
//    Hank Childs, Fri Mar  9 14:41:18 PST 2001
//    Made all databases return a SIL.
//
//    Kathleen Bonnell, Mon Apr  9 14:47:12 PDT 2001 
//    Reflect that avtDomainTree replaced by avtDataTree.
//
//    Hank Childs, Tue May  1 12:53:10 PDT 2001
//    Added ClearCache.
//
//    Hank Childs, Fri Aug 17 15:48:46 PDT 2001
//    Removed dependences on avtDataset so this can serve up general
//    avtDataObjects.
//
//    Hank Childs, Mon Dec  3 09:50:18 PST 2001
//    Allow a database to be marked as for meta-data only, so it doesn't
//    do as much computation when it is read in.
//
//    Kathleen Bonnell, Wed Dec 12 10:21:13 PST 2001 
//    Added pure virtual method "Query". 
//
//    Sean Ahern, Tue May 21 11:58:02 PDT 2002
//    Added a virtual method for freeing up resources.
//
//    Jeremy Meredith, Tue Aug 27 15:18:21 PDT 2002
//    Added GetFileListFromTextFile.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Query no longer virtual.  Added virtual QueryScalars/Vectors/Material.
//
//    Kathleen Bonnell, Fri Dec  6 16:25:20 PST 2002 
//    Added virtual QueryNodes.
//
//    Kathleen Bonnell, Fri Dec 27 14:09:40 PST 2002 
//    Added arguments to QueryNodes.
//
//    Kathleen Bonnell, Fri Apr 18 14:11:24 PDT 2003  
//    Added virtual QueryMesh.
//
//    Brad Whitlock, Wed May 14 09:08:32 PDT 2003
//    I added an optional timeState argument to GetMetaData and GetSIL.
//
//    Jeremy Meredith, Wed Jun 11 16:39:27 PDT 2003
//    Added an option argument to PopulateDataObjectInformation.
//
//    Kathleen Bonnell, Fri Jun 20 13:52:00 PDT 2003  
//    Added QueryZones, added parameter to other Query methods.
//
//    Kathleen Bonnell, Tue Sep  9 16:51:10 PDT 2003 
//    Changed PickVarInfo argument in QueryMesh to std::string.
//
//    Hank Childs, Mon Sep 22 09:20:08 PDT 2003
//    Add support for picking tensors.
//
//    Mark C. Miller, 29Sep03
//    Added support for time-varying SIL/MetaData
//    Made timestep argument required for GetMetaData and GetSIL 
//
//    Kathleen Bonnell, Tue Nov 18 14:07:13 PST 2003 
//    Added bool and stringVector args to QueryNodes, QueryZones, in
//    support of logical zone coordinates. 
// 
//    Kathleen Bonnell, Thu Nov 20 15:17:21 PST 2003 
//    Added QuerySpecies. 
//
//    Kathleen Bonnell, Thu Nov 20 17:47:57 PST 2003 
//    Added virtual FindElementForPoint, defined here so derived types don't
//    have to. 
//    
//    Hank Childs, Tue Nov 25 07:39:32 PST 2003
//    Added AddMeshQualityExpressions.
//
//    Kathleen Bonnell, Wed Dec 17 14:58:31 PST 2003 
//    Updated arguments lists for QueryNodes and QueryZones so that multiple
//    types of coordinates could be retrieved. 
//
//    Kathleen Bonnell, Mon Dec 22 14:39:30 PST 2003 
//    Added virtual GetDomainName, defined here so derived types don't
//    have to. 
//
//    Mark C. Miller, Tue Mar 16 09:38:19 PST 2004
//    Added ActivateTimestep method
//
//    Mark C. Miller, Tue Mar 16 14:40:19 PST 2004
//    Added stateIndex argument to GetIOInformation.
//    Implemented PopulateIOInformation here instead of in file
//
//    Jeremy Meredith/Hank Childs, Tue Mar 23 12:26:55 PST 2004
//    Add file format as a data member.
//
//    Kathleen Bonnell, Tue May 25 16:16:25 PDT 2004 
//    Added virtual QueryZoneCenter, defined here so derived types don't
//    have to. 
//
//    Kathleen Bonnell, Wed Jun  9 12:44:48 PDT 2004 
//    Added bool arg to QueryMesh. 
//
//    Kathleen Bonnell, Thu Jun 10 18:15:11 PDT 2004
//    Rename QueryZoneCenter to QueryCoords, added bool arg. 
//
//    Jeremy Meredith, Tue Aug 24 17:58:07 PDT 2004
//    Added ability to clear the metadata and SIL caches.  This was needed
//    for simulations.
//
//    Kathleen Bonnell, Thu Sep 23 17:48:37 PDT 2004 
//    Added args to QueryZones and QueryNodes, to support ghost-element 
//    indication. 
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added argument for selections applied to PopulateDataObjectInformation
//
//    Mark C. Miller, Tue Oct 19 14:08:56 PDT 2004
//    Changed 'spec' arg of PopulateDataObjectInformation to a ref_ptr
//
//    Mark C. Miller, Wed Oct 20 10:35:53 PDT 2004
//    Added GetExtentsFromAuxiliaryData
//
//    Kathleen Bonnell, Wed Dec 15 08:31:38 PST 2004 
//    Changed std::vector<std::string> to stringVector, std::vector<int> to
//    intVector.  Added 'QueryGlobalIds'. 
//
//    Kathleen Bonnell, Wed Dec 15 17:34:25 PST 2004 
//    Added 'LocalIdForGlobal'.
//
//    Kathleen Bonnell, Thu Dec 16 17:10:43 PST 2004 
//    Added another bool arg to QueryCoords.
//
//    Kathleen Bonnell, Tue Jan 25 07:59:28 PST 2005 
//    Added const char* arg to QueryCoords. 
//
//    Hank Childs, Sun Feb 27 11:20:39 PST 2005
//    Added argument to CanDoDynamicLoadBalancing.
//
//    Brad Whitlock, Mon Apr 4 11:46:10 PDT 2005
//    Added QueryLabels.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added forceReadAllCyclesAndTimes bool to MetaData methods
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added forceReadThisStateCycleTime bool to GetMetaData
//    Added method SetCycleTimeInDatabaseMetaData
//
//    Hank Childs, Tue Jul 19 15:52:47 PDT 2005
//    Add QueryArray.
//
//    Hank Childs, Fri Oct  7 08:21:03 PDT 2005
//    Added fullDBName.
//
//    Hank Childs, Wed Jan 11 09:15:23 PST 2006
//    Add AddTimeDerivativeExpressions.
//
//    Mark C. Miller, Tue Aug 15 15:28:11 PDT 2006
//    Added static domain decomposition help functions
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//    Added treatAllDBsAsTimeVarying to SIL methods
//
//    Cyrus Harrison, Fri Sep 14 13:59:30 PDT 2007
//    Added floating point format argument to QueryNodes and QueryZones
//
//    Cyrus Harrison, Wed Nov 28 10:11:41 PST 2007
//    Added AddVectorMagnitudeExpressions.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Mark C. Miller, Tue Jun 10 22:36:25 PDT 2008
//    Added support for ignoring bad extents from dbs.
//
//    Mark C. Miller, Mon Feb  9 14:15:05 PST 2009
//    Added method to convert 1D Var MDs to Curve MDs in metadata.
//
//    Jeremy Meredith, Fri Jan  8 16:15:02 EST 2010
//    Added ability to turn on stricter file format error checking.
//
// ****************************************************************************

class DATABASE_API avtDatabase
{
  public:
                                avtDatabase();
    virtual                    ~avtDatabase();

    virtual const char         *GetFilename(int) { return NULL; };
    const std::string          &GetFullDBName(void) { return fullDBName; };
    void                        SetFullDBName(const std::string &f)
                                                    { fullDBName = f; };

    avtDataObject_p             GetOutput(const char *, int);

    virtual void                GetAuxiliaryData(avtDataRequest_p,
                                                VoidRefList &,
                                                const char *type,void *args)=0;

    avtDatabaseMetaData        *GetMetaData(int stateIndex,
                                    bool forceReadAllCyclesTimes = false,
                                    bool forceReadThisStateCycleTime = false,
                                    bool treatAllDBsAsTimeVarying = false);
    avtSIL                     *GetSIL(int stateIndex,
                                    bool treatAllDBsAsTimeVarying = false);
    int                         GetMostRecentTimestep() const;

    virtual void                ActivateTimestep(int stateIndex) {;}; 
    virtual void                PopulateIOInformation(int stateIndex,
                                    avtIOInformation& ioInfo) {;};

    virtual void                ClearCache(void);
    virtual void                FreeUpResources(void);
    virtual bool                MetaDataIsInvariant(void);
    virtual bool                SILIsInvariant(void);
    virtual bool                CanDoStreaming(avtDataRequest_p);
    virtual int                 NumStagesForFetch(avtDataRequest_p);

    const avtIOInformation     &GetIOInformation(int stateIndex);

    static bool                 OnlyServeUpMetaData(void)
                                     { return onlyServeUpMetaData; };
    static void                 SetOnlyServeUpMetaData(bool val)
                                     { onlyServeUpMetaData = val; };
 
    void                        Query(PickAttributes *);
    virtual bool                FindElementForPoint(const char *, const int, 
                                    const int, const char *, double[3], int &)
                                    { return false; } ;
    virtual bool                QueryCoords(const std::string &, const int, 
                                    const int, const int, double[3], const bool,
                                    const bool, const char *mn = NULL)
                                    { return false; } ;

    virtual void                GetDomainName(const std::string &, const int ts,
                                    const int dom, std::string &) {;};

    static void                 GetFileListFromTextFile(const char *,
                                                        char **&, int &);

    void                        SetFileFormat(const std::string &ff)
                                      { fileFormat = ff; };
    const std::string          &GetFileFormat(void) const 
                                      { return fileFormat; };

    void                        ClearMetaDataAndSILCache();

    void                        SetIgnoreExtents(bool set)
                                    { ignoreExtents = set; };
    bool                        GetIgnoreExtents() const
                                    { return ignoreExtents; };

    virtual void                SetStrictMode(bool) { }

    // methods useful for decomposing rectlinear data on the fly during read
    static double               RectilinearDecompCost(int i, int j, int k,
                                    int nx, int ny, int nz);
    static double               ComputeRectilinearDecomposition(int ndims,
                                    int n, int nx, int ny, int nz,
                                    int *imin, int *jmin, int *kmin);
    static double               ComputeRectilinearSpatialDecomposition(int ndims,
                                    int n, double wx, double wy, double wz,
                                    int *imin, int *jmin, int *kmin);
    static void                 ComputeDomainLogicalCoords(int dataDim,
                                    int domCount[3], int rank,
                                    int domLogicalCoords[3]);
    static void                 ComputeDomainBounds(int globalZoneCount, int domCount,
                                    int domLogicalCoord, int *globalZoneStart, int *zoneCount);
    static void                 ComputeDomainSpatialBounds(double globalWidth, int domCount,
                                    int domLogicalCoord, double *globalStart, double *width);

  protected:
    std::list<CachedMDEntry>               metadata;
    std::list<CachedSILEntry>              sil;
    std::vector<avtDataObjectSource *>     sourcelist;
    avtIOInformation                       ioInfo;
    bool                                   gotIOInfo;
    static bool                            onlyServeUpMetaData;
    std::string                            fileFormat;
    std::string                            fullDBName;

    static int                             mdCacheSize;
    static int                             silCacheSize;

    bool                                  *invariantMetaData;
    bool                                  *invariantSIL;

    bool                                   ignoreExtents;

    void                        GetNewMetaData(int stateIndex,
                                    bool forceReadAllCyclesTimes = false);
    void                        GetNewSIL(int stateIndex,
                                    bool treatAllDBsAsTimeVarying = false);
    void                        AddMeshQualityExpressions(avtDatabaseMetaData *);
    void                        AddTimeDerivativeExpressions(avtDatabaseMetaData *);
    void                        AddVectorMagnitudeExpressions(avtDatabaseMetaData *);
    void                        Convert1DVarMDsToCurveMDs(avtDatabaseMetaData *);
    
    virtual bool                HasInvariantMetaData(void) const = 0;
    virtual bool                HasInvariantSIL(void) const = 0;

    virtual avtDataObjectSource *CreateSource(const char *, int) = 0;
    virtual void                SetCycleTimeInDatabaseMetaData(avtDatabaseMetaData *, int) = 0;
    virtual void                SetDatabaseMetaData(avtDatabaseMetaData *,
                                    int=0, bool=false) = 0;
    virtual void                PopulateSIL(avtSIL *, int=0,
                                    bool treatAllDBsAsTimeVarying = false) = 0;

    void                        PopulateDataObjectInformation(avtDataObject_p&,
                                                  const char *,
                                                  int,
                                                  const vector<bool> &selsApplied,
                                                  avtDataRequest_p =NULL);
    bool                        GetExtentsFromAuxiliaryData(avtDataRequest_p spec,
                                                            const char *var,
                                                            const char *type,
                                                            double *extents);
    virtual bool                QueryScalars(const std::string &, const int, 
                                             const int, const int,
                                             const intVector &,
                                             PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QueryVectors(const std::string &, const int, 
                                             const int, const int,
                                             const intVector &,
                                             PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QueryTensors(const std::string &, const int, 
                                             const int, const int,
                                             const intVector &,
                                             PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QuerySymmetricTensors(const std::string &,
                                             const int, const int, const int,
                                             const intVector &,
                                             PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QueryLabels(const std::string &, const int, 
                                             const int, const int,
                                             const intVector &,
                                             PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QueryArrays(const std::string &, const int, 
                                             const int, const int,
                                             const intVector &,
                                             PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QueryMaterial(const std::string &, const int, 
                                              const int, const int,
                                              const intVector &,
                                              PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QuerySpecies(const std::string &, const int, 
                                             const int, const int,
                                             const intVector &,
                                             PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QueryNodes(const std::string &, const int, 
                                           const std::string &,
                                           const int, bool &, const int,
                                           intVector &, intVector &, 
                                           const bool, double [3],
                                           const int, const bool, const bool,
                                           const bool, stringVector &,
                                           stringVector &, stringVector &,
                                           const bool,  const bool,
                                           stringVector &, stringVector &)
                                               {return false; };
    virtual bool                QueryMesh(const std::string &, const int, const int, 
                                          std::string &, const bool) {return false; };

    virtual bool                QueryZones(const std::string &, const int,
                                           const std::string &, int &,
                                           bool &, const int, intVector &, 
                                           intVector &, const bool,
                                           double [3], const int, const bool, 
                                           const bool,  const bool, 
                                           stringVector &, stringVector &, 
                                           stringVector &, const bool, const bool, 
                                           stringVector &, stringVector &)
                                               { return false; } ;

    virtual void               QueryGlobalIds(const int, const std::string &,
                                        const int, const bool, const int, 
                                        const intVector &, int &, intVector &){ ; };

    virtual int                LocalIdForGlobal(const int, const std::string &,
                                        const int, const bool, const int)
                                        { return -1; };

 
};


#endif


