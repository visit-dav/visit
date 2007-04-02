// ************************************************************************* //
//                            avtPickQuery.h                                 //
// ************************************************************************* //

#ifndef AVT_PICK_QUERY_H
#define AVT_PICK_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

#include <PickAttributes.h>
#include <avtTypes.h>

class avtMatrix;
class vtkDataSet;
class avtExpressionEvaluatorFilter;


// ****************************************************************************
//  Class: avtPickQuery
//
//  Purpose:
//    This query gathers information about a cell picked by the user. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 17 09:39:19 PDT 2003  
//    Added member invTransform.
//
//    Kathleen Bonnell, Wed May  7 13:24:37 PDT 2003 
//    Renamed 'LocateCell' as 'LocatorFindCell'. Added 'RGridFindCell'. 
//
//    Kathleen Bonnell, Wed Jun 18 18:07:55 PDT 2003   
//    Add member ghostType.
//
//    Kathleen Bonnell, Fri Jun 27 17:06:27 PDT 2003  
//    Add methods RetrieveNodes, RetrieveZones, DeterminePickedNode,
//    SetRealIds, RetrieveVarInfo -- to support Node Pick and make code
//    more legible.
//
//    Kathleen Bonnell, Thu Nov 13 09:05:14 PST 2003 
//    Remove RgridFindCell, LocatorFindCell -- functionality now resides
//    in vtkVisItUtility. 
//    
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Kathleen Bonnell, Mon Mar  8 15:39:15 PST 2004 
//    Added SetNeedTransform / SetTransform and setUseSet. 
//    
//    Kathleen Bonnell, Thu Apr  1 09:21:22 PST 2004 
//    Inherit from avtVariableQuery, moved common methods to parent class 
//    (RetrieveNodes, RetrieveZones, RetrieveVarInfo). 
// 
//    Kathleen Bonnell, Thu Jul 29 17:10:48 PDT 2004 
//    No longer inherit from avtVariableQuery, moved common methods back to
//    this class: (RetrieveNodes, RetrieveZones, RetrieveVarInfo). 
//
//    Kathleen Bonnell, Wed Aug 11 09:21:07 PDT 2004 
//    Added RetrieveVarInfo method with extra args.  Added two
//    GetCurrentZoneForOriginal methods. 
// 
//    Kathleen Bonnell, Mon Aug 30 17:56:29 PDT 2004
//    Added skippedLocate flag, and Set method.
// 
//    Kathleen Bonnell, Fri Sep  3 08:36:58 PDT 2004 
//    Removed VerifyInput. 
// 
//    Kathleen Bonnell, Thu Sep 23 17:38:15 PDT 2004 
//    Removed SetRealIds. 
// 
//    Kathleen Bonnell, Wed Dec 15 09:19:39 PST 2004 
//    Add SetGlobalIds. 
// 
//    Kathleen Bonnell, Wed Dec 15 17:16:17 PST 2004 
//    Add GetCurrentNodeForOriginal.
// 
//    Kathleen Bonnell, Wed May 11 17:50:53 PDT 2005 
//    Added ConvertElNamesToGlobal.
//
//    Kathleen Bonnell, Tue Jun 28 10:47:35 PDT 2005 
//    Re-added SetRealIds. 
// 
// ****************************************************************************

class QUERY_API avtPickQuery : public avtDatasetQuery
{
  public:
                                    avtPickQuery();
    virtual                        ~avtPickQuery();


    virtual const char             *GetType(void)
                                             { return "avtPickQuery"; };
    virtual const char             *GetDescription(void)
                                             { return "Picking"; };

    virtual bool                    OriginalData(void) { return true; };

    void                            SetPickAtts(const PickAttributes *pa);
    const PickAttributes *          GetPickAtts(void);
    virtual void                    SetTransform(const avtMatrix *m){}; 
    virtual void                    SetInvTransform(const avtMatrix *m){};
    void                            SetNeedTransform(const bool b)
                                        { needTransform = b; };
    void                            SetSkippedLocate(const bool s)
                                        { skippedLocate = s; };

  protected:
    int                             cellOrigin;
    int                             blockOrigin;
    avtGhostType                    ghostType;
    const avtMatrix                *transform;
    bool                            singleDomain;
    bool                            needTransform;
    bool                            skippedLocate;

    // Query-specific code that needs to be defined.
    virtual void                    PreExecute(void);
    virtual void                    PostExecute(void);
    virtual avtDataObject_p         ApplyFilters(avtDataObject_p);   

    // Pick-specific code
    virtual void                    Preparation(void){};
    bool                            DeterminePickedNode(vtkDataSet *, int &);
    void                            GetNodeCoords(vtkDataSet *, const int);
    void                            GetZoneCoords(vtkDataSet *, const int);

    void                            RetrieveVarInfo(vtkDataSet *);   
    void                            RetrieveVarInfo(vtkDataSet *, const int, 
                                                    const intVector &);   
    bool                            RetrieveNodes(vtkDataSet *, int);   
    bool                            RetrieveZones(vtkDataSet *, int);   

    int                             GetCurrentNodeForOriginal(vtkDataSet *, 
                                                              const int);
    int                             GetCurrentZoneForOriginal(vtkDataSet *, 
                                                              const int);
    intVector                       GetCurrentZoneForOriginal(vtkDataSet *, 
                                                              const intVector&);
    void                            SetGlobalIds(vtkDataSet *, int);
                                                              
    void                            ConvertElNamesToGlobal(void);
    void                            SetRealIds(vtkDataSet *);

    PickAttributes                  pickAtts;
    avtExpressionEvaluatorFilter   *eef;
    avtQueryableSource             *src;
};


#endif


