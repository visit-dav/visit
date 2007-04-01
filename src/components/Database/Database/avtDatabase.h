// ************************************************************************* //
//                                 avtDatabase.h                             //
// ************************************************************************* //

#ifndef AVT_DATABASE_H
#define AVT_DATABASE_H

#include <database_exports.h>

#include <vector>
#include <list>

#include <void_ref_ptr.h>

#include <avtDataSpecification.h>
#include <avtDataset.h>
#include <avtIOInformation.h>
#include <avtTypes.h>


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
// ****************************************************************************

class DATABASE_API avtDatabase
{
  public:
                                avtDatabase();
    virtual                    ~avtDatabase();

    virtual const char         *GetFilename(int) { return NULL; };
    avtDataObject_p             GetOutput(const char *, int);

    virtual void                GetAuxiliaryData(avtDataSpecification_p,
                                                VoidRefList &,
                                                const char *type,void *args)=0;

    avtDatabaseMetaData        *GetMetaData(int stateIndex);
    avtSIL                     *GetSIL(int stateIndex);
    int                         GetMostRecentTimestep() const;

    virtual void                ActivateTimestep(int stateIndex) {;}; 
    virtual void                PopulateIOInformation(int stateIndex,
                                    avtIOInformation& ioInfo) {;};

    virtual void                ClearCache(void);
    virtual void                FreeUpResources(void);
    virtual bool                CanDoDynamicLoadBalancing(void);
    virtual bool                MetaDataIsInvariant(void);
    virtual bool                SILIsInvariant(void);
    virtual int                 NumStagesForFetch(avtDataSpecification_p);

    const avtIOInformation     &GetIOInformation(int stateIndex);

    static bool                 OnlyServeUpMetaData(void)
                                     { return onlyServeUpMetaData; };
    static void                 SetOnlyServeUpMetaData(bool val)
                                     { onlyServeUpMetaData = val; };
 
    void                        Query(PickAttributes *);
    virtual bool                FindElementForPoint(const char *, const int, 
                                    const int, const char *, float[3], int &)
                                    { return false; } ;

    virtual void                GetDomainName(const std::string &, const int ts,
                                    const int dom, std::string &) {;};

    static void                 GetFileListFromTextFile(const char *,
                                                        char **&, int &);
  protected:
    std::list<CachedMDEntry>               metadata;
    std::list<CachedSILEntry>              sil;
    std::vector<avtDataObjectSource *>     sourcelist;
    avtIOInformation                       ioInfo;
    bool                                   gotIOInfo;
    static bool                            onlyServeUpMetaData;

    static int                             mdCacheSize;
    static int                             silCacheSize;

    bool                                  *invariantMetaData;
    bool                                  *invariantSIL;

    void                        GetNewMetaData(int stateIndex);
    void                        GetNewSIL(int stateIndex);
    void                        AddMeshQualityExpressions(avtDatabaseMetaData *);

    virtual bool                HasInvariantMetaData(void) const = 0;
    virtual bool                HasInvariantSIL(void) const = 0;

    virtual avtDataObjectSource *CreateSource(const char *, int) = 0;
    virtual void                SetDatabaseMetaData(avtDatabaseMetaData *,int=0) = 0;
    virtual void                PopulateSIL(avtSIL *, int=0) = 0;

    void                        PopulateDataObjectInformation(avtDataObject_p&,
                                                  const char *,
                                                  int,
                                                  avtDataSpecification* =NULL);
    virtual bool                QueryScalars(const std::string &, const int, 
                                             const int, const int,
                                             const std::vector<int> &,
                                             PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QueryVectors(const std::string &, const int, 
                                             const int, const int,
                                             const std::vector<int> &,
                                             PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QueryTensors(const std::string &, const int, 
                                             const int, const int,
                                             const std::vector<int> &,
                                             PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QuerySymmetricTensors(const std::string &,
                                             const int, const int, const int,
                                             const std::vector<int> &,
                                             PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QueryMaterial(const std::string &, const int, 
                                              const int, const int,
                                              const std::vector<int> &,
                                              PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QuerySpecies(const std::string &, const int, 
                                             const int, const int,
                                             const std::vector<int> &,
                                             PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QueryNodes(const std::string &, const int, 
                                           const int, const int,
                                           std::vector<int> &, float [3],
                                           const int, const bool, const bool,
                                           const bool, std::vector<std::string> &,
                                           std::vector<std::string> &,
                                           std::vector<std::string> &,
                                           const bool,  const bool,
                                           std::vector<std::string> &,
                                           std::vector<std::string> &)
                                               {return false; };
    virtual bool                QueryMesh(const std::string &, const int, const int, 
                                          std::string &) {return false; };

    virtual bool                QueryZones(const std::string &,const int,int &,
                                           const int, std::vector<int> &, 
                                           float [3], const int, const bool, 
                                           const bool,  const bool, 
                                           std::vector<std::string> &,
                                           std::vector<std::string> &,
                                           std::vector<std::string> &,
                                           const bool, const bool, 
                                           std::vector<std::string> &,
                                           std::vector<std::string> &)
                                               { return false; } ;
};


#endif


