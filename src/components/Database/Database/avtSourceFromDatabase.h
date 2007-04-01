// ************************************************************************* //
//                           avtSourceFromDatabase.h                         //
// ************************************************************************* //

#ifndef AVT_SOURCE_FROM_DATABASE_H
#define AVT_SOURCE_FROM_DATABASE_H
#include <database_exports.h>


#include <avtTerminatingDatasetSource.h>
#include <avtDataTree.h>


class  vtkDataSet;

class  avtDatasetDatabase;
class  PickAttributes;


// ****************************************************************************
//  Class: avtSourceFromDatabase
//
//  Purpose:
//      A source object (pipeline terminator) that is created from an avt
//      database.
//
//  Programmer: Hank Childs
//  Creation:   August 9, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Tue Sep 26 09:42:26 PDT 2000
//    Made FetchDomain return an avtDomain.
//
//    Kathleen Bonnell, Tue Dec 19 09:06:23 PST 2000 
//    Renamed avtTerminatingSource as avtTerminatingDatasetSource. 
//
//    Kathleen Bonnell, Fri Feb  9 16:38:40 PST 2001 
//    Made FetchDomain return avtDomainTree_p. 
//
//    Hank Childs, Thu Mar  1 11:20:47 PST 2001
//    Modified routine to return all of the domain at once instead of one at
//    a time.
//
//    Hank Childs, Tue Apr 10 16:13:07 PDT 2001
//    Added GetMaterialAuxiliaryData.
//
//    Kathleen Bonnell, Wed Apr 11 08:44:46 PDT 2001 
//    Changed return type of FetchDomains to be avtDataTree_p. 
//
//    Hank Childs, Tue Jun 19 21:54:53 PDT 2001
//    Overhauled interface to reflect changes in the base class.
//
//    Hank Childs, Thu Oct 25 16:49:26 PDT 2001
//    Allow for databases that don't do dynamic load balancing.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Add Query method. 
//
//    Kathleen Bonnell, Thu Nov 13 08:44:01 PST 2003 
//    Add FindElementForPoint method. 
//
//    Kathleen Bonnell, Mon Dec 22 14:39:30 PST 2003
//    Add GetDomainName method. 
//
//    Kathleen Bonnell, Tue May 25 16:16:25 PDT 2004 
//    Add QueryZoneCenter method. 
//
//    Jeremy Meredith, Wed Jun  9 09:12:24 PDT 2004
//    Added species aux data.
//
//    Kathleen Bonnell, Thu Jun 10 18:15:11 PDT 2004 
//    Rename QueryZoneCenter to QueryCoords, added bool arg.
//
//    Kathleen Bonnell, Thu Dec 16 17:11:19 PST 2004 
//    Added another bool arg to QueryCoords. 
//
// ****************************************************************************

class DATABASE_API avtSourceFromDatabase : public avtTerminatingDatasetSource
{
  public:
                        avtSourceFromDatabase(avtDatasetDatabase *, 
                                              const char *, int);
    virtual            ~avtSourceFromDatabase();

    virtual void        FetchMeshAuxiliaryData(const char *type, void *args,
                                        avtDataSpecification_p, VoidRefList &);
    virtual void        FetchVariableAuxiliaryData(const char *type,
                            void *args, avtDataSpecification_p, VoidRefList &);
    virtual void        FetchMaterialAuxiliaryData(const char *type, 
                            void *args, avtDataSpecification_p, VoidRefList &);
    virtual void        FetchSpeciesAuxiliaryData(const char *type, 
                            void *args, avtDataSpecification_p, VoidRefList &);

    virtual avtDataSpecification_p
                        GetFullDataSpecification(void);

    void                InvalidateZones(void);
    void                DatabaseProgress(int, int, const char *);
    virtual void        Query(PickAttributes *);
    virtual bool        QueryCoords(const std::string &, const int, const int,
                                    const int, float[3], const bool, const bool);
    virtual bool        FindElementForPoint(const char *, const int, 
                            const int, const char *, float[3], int &);
    virtual void        GetDomainName(const std::string &, const int, 
                            const int, std::string &);

  protected:
    avtDatasetDatabase      *database;
    char                    *variable;
    int                      timestep;
    avtDataSpecification_p   lastSpec;

    virtual bool        FetchDataset(avtDataSpecification_p, avtDataTree_p &);
    virtual bool        CanDoDynamicLoadBalancing(void);
    virtual int         NumStagesForFetch(avtDataSpecification_p);
};


#endif


