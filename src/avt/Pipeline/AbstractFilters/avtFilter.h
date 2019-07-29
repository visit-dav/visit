// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtFilter.h                                 //
// ************************************************************************* //

#ifndef AVT_FILTER_H
#define AVT_FILTER_H

#include <pipeline_exports.h>

#include <vector>

#include <void_ref_ptr.h>

#include <avtDataObjectSource.h>
#include <avtDataObjectSink.h>


class     AttributeGroup;

class     avtDynamicAttribute;
class     avtMetaData;
class     avtWebpage;


// ****************************************************************************
//  Method: avtFilter
//
//  Purpose:
//      A filter is an object that does not originate or terminate a pipeline.
//      It defines what it looks like to propagate an Update to an upstream
//      filter and also incorporates the idea of "Executes", which are
//      defined by all derived types of filters.
// 
//  Programmer: Hank Childs
//  Creation:   May 30, 2001
//
//  Modifications:
//  
//    Kathleen Bonnell, Wed Oct  3 08:53:21 PDT 2001
//    Added TryCurrent and GetCurrent Data/Spatial Extents.
//
//    Hank Childs, Wed Oct 24 14:21:18 PDT 2001
//    Added PreExecute and PostExecute from avtSIMODataTreeIterator.
//
//    Hank Childs, Fri Mar 15 15:25:33 PST 2002 
//    Add support for attributes.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Fri Dec  3 14:22:42 PST 2004
//    Add variable name argument to SearchDataForDataExtents.
//
//    Hank Childs, Tue Jun  7 14:55:40 PDT 2005
//    Add friend status to the facade filter.
//
//    Hank Childs, Thu Dec 21 09:17:43 PST 2006
//    Add support for debug dumps.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//    All filters can now report that they understand these grids, though
//    the virtual default in this base class is that each does not.
//
//    Hank Childs, Fri Jun 15 11:37:59 PDT 2007
//    Added a static data member to keep track of the number of filters
//    currently executing (which also informs the amount of sub-pipelines
//    that are executing, which allows for formatting the debug dump).
//
//    Cyrus Harrison, Wed Feb 13 10:19:20 PST 2008
//    Removed debug dump flag b/c this is now handled by avtDebugDumpOptions.
//
//    Hank Childs, Mon Feb 23 13:20:07 PST 2009
//    Added support for named selections.
//
//    Hank Childs, Mon Apr  6 12:58:50 PDT 2009
//    Add another method for handling named selections.
//
//    Tom Fogal, Tue Jun 23 20:21:15 MDT 2009
//    Made spatial extent methods const.
//
//    Hank Childs, Sun Nov 28 06:19:25 PST 2010
//    Add support for caching arbitrary data structures in the database.
//
//    Hank Childs, Tue Nov 30 20:38:36 PST 2010
//    Add method SearchDataForSpatialExtents.
//
//    Brad Whitlock, Wed Mar 19 13:14:49 PDT 2014
//    I added a callback mechanism that extends UpdateDataObjectInfo so that
//    filter facades can correctly affect the output data object info.
//    Work partially supported by DOE Grant SC0007548.
//
//    Kathleen Biagas, Mon Jun  5 16:46:14 PDT 2017
//    Added ResetAllExtents.
//
// ****************************************************************************

class PIPELINE_API avtFilter
    : virtual public avtDataObjectSource, virtual public avtDataObjectSink
{
    friend class                        avtFacadeFilter;

  public:
                                        avtFilter();
    virtual                            ~avtFilter();

    virtual const char                 *GetType(void) = 0;
    virtual const char                 *GetDescription(void) { return NULL; };

    virtual bool                        Equivalent(const AttributeGroup *)
                                            { return false; };

    virtual bool                        Update(avtContract_p);
    virtual void                        ResetAllExtents(void);

    virtual avtOriginatingSource       *GetOriginatingSource(void);
    virtual avtQueryableSource         *GetQueryableSource(void);
    avtContract_p                       GetGeneralContract(void);
    virtual void                        ReleaseData(void);

    virtual avtNamedSelection          *CreateNamedSelection(avtContract_p,
                                                             const std::string &);
    // This method is needed for filters that bypass the pipeline to talk
    // directly to the database and also need to know what named selections
    // are being used.
    virtual void                        RegisterNamedSelection(const std::string &) {;};

    void                                SetUpdateDataObjectInfoCallback(
                                            void (*updateDOI)(avtDataObject_p &input,
                                                              avtDataObject_p &output,
                                                              void *),
                                            void *updateDOIData);

  protected:
    bool                                modified;
    bool                                inExecute;
    void                               (*updateDOI)(avtDataObject_p &input,
                                                    avtDataObject_p &output,
                                                    void *);
    void                               *updateDOIData;

    static int                          numInExecute;
    avtWebpage                         *webpage;
    std::vector<avtDynamicAttribute *>  dynamicAttributes;

    virtual void                        Execute(void) = 0;
    avtContract_p                       ModifyContractAndDoBookkeeping
                                                              (avtContract_p);
    virtual avtContract_p               ModifyContract(avtContract_p);

    virtual void                        ChangedInput(void);
    virtual void                        InitializeFilter(void);
    virtual void                        VerifyInput(void);
    virtual int                         AdditionalPipelineFilters(void);

    void                                PassOnDataObjectInfo(void);
    virtual void                        UpdateDataObjectInfo(void);

    virtual bool                        FilterUnderstandsTransformedRectMesh();

    virtual void                        PreExecute(void);
    virtual void                        PostExecute(void);
    virtual void                        ExamineContract(avtContract_p);

    avtMetaData                        *GetMetaData(void);

    void                                UpdateProgress(int, int);

    bool                                TryDataExtents(double *,
                                                       const char * = NULL);
    void                                GetDataExtents(double *,
                                                       const char * = NULL);
    bool                                TrySpatialExtents(double *) const;
    void                                GetSpatialExtents(double *);
    bool                                TryActualDataExtents(double *);
    void                                GetActualDataExtents(double *);
    bool                                TryActualSpatialExtents(double *);
    void                                GetActualSpatialExtents(double *);
    virtual void                        SearchDataForDataExtents(double *,
                                                                 const char *);
    virtual void                        SearchDataForSpatialExtents(double *);

    void                                RegisterDynamicAttribute(
                                                        avtDynamicAttribute *);
    void                                ResolveDynamicAttributes(void);

    void                                DumpDataObject(avtDataObject_p,
                                                      const char *);
    void                                DumpContract(avtContract_p,
                                                     const char *);
    void                                InitializeWebpage(void);
    void                                FinalizeWebpage(void);

    enum CacheItemDependence
    {
        DATA_DEPENDENCE = 1,          // Example: scalar tree for contouring
        SPATIAL_DEPENDENCE = 2,       // Example: lookup structure for integral curves
        CONNECTIVITY_DEPENDENCE = 4   // Example: facelist (point positions not important)
    };

    bool                                CheckDependencies(int);
    bool                                CanCacheDataItem(void);
    bool                                CanCacheSpatialItem(void);
    bool                                CanCacheConnectivityItem(void);
    void_ref_ptr                        FetchArbitraryRefPtr(
                                                      int dependencies,
                                                      const char *name,
                                                      int dom, int ts,
                                                      const char *type);
    void                                StoreArbitraryRefPtr(
                                                      int dependencies,
                                                      const char *name,
                                                      int dom, int ts,
                                                      const char *type,
                                                      void_ref_ptr);
};

#endif


