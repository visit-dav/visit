// ************************************************************************* //
//                                 avtDatabase.h                             //
// ************************************************************************* //

#ifndef AVT_DATABASE_H
#define AVT_DATABASE_H

#include <database_exports.h>

#include <vector>

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


// ****************************************************************************
//  Class: avtDatabase
//
//  Purpose:
//      Provides an interface for what our database looks like.  Derived types
//      should have no public functions besides constructors and destructors.
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

    avtDatabaseMetaData        *GetMetaData(int timeState = 0);
    avtSIL                     *GetSIL(int timeState = 0);

    virtual void                ClearCache(void);
    virtual void                FreeUpResources(void);
    virtual bool                CanDoDynamicLoadBalancing(void);
    virtual int                 NumStagesForFetch(avtDataSpecification_p);

    const avtIOInformation     &GetIOInformation(void);

    static bool                 OnlyServeUpMetaData(void)
                                     { return onlyServeUpMetaData; };
    static void                 SetOnlyServeUpMetaData(bool val)
                                     { onlyServeUpMetaData = val; };
 
    void                        Query(PickAttributes *);

    static void                 GetFileListFromTextFile(const char *,
                                                        char **&, int &);
  protected:
    avtDatabaseMetaData                   *metadata;
    avtSIL                                *sil;
    std::vector<avtDataObjectSource *>     sourcelist;
    avtIOInformation                       ioInfo;
    bool                                   gotIOInfo;
    static bool                            onlyServeUpMetaData;

    virtual avtDataObjectSource *CreateSource(const char *, int) = 0;
    virtual void                SetDatabaseMetaData(avtDatabaseMetaData *,int=0) = 0;
    virtual void                PopulateSIL(avtSIL *, int=0) = 0;
    virtual void                PopulateIOInformation(avtIOInformation &);

    void                        PopulateDataObjectInformation(avtDataObject_p&,
                                                  const char *,
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
    virtual bool                QueryMaterial(const std::string &, const int, 
                                              const int, const int,
                                              const std::vector<int> &,
                                              PickVarInfo &, const bool) 
                                                  {return false; };
    virtual bool                QueryNodes(const std::string &, const int, 
                                           const int, const int,
                                           std::vector<int> &, float [3],
                                           const int, const bool, const bool,
                                           std::vector<std::string> &)
                                               {return false; };
    virtual bool                QueryMesh(const std::string &, const int, 
                                          PickVarInfo &) {return false; };

    virtual bool                QueryZones(const std::string &, const int, int &, 
                                           const int, std::vector<int> &, 
                                           float [3], const int, const bool, 
                                           const bool, std::vector<std::string> &)
                                               { return false; } ;
};


#endif


