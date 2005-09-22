// ************************************************************************* //
//                            avtOnionPeelFilter.h                           //
// ************************************************************************* //

#ifndef AVT_ONION_PEEL_FILTER_H
#define AVT_ONION_PEEL_FILTER_H


#include <avtPluginStreamer.h>
#include <OnionPeelAttributes.h>


class vtkDataSet;
class vtkOnionPeelFilter;
class vtkPolyDataOnionPeelFilter;


// ****************************************************************************
//  Class: avtOnionPeelFilter
//
//  Purpose:
//    A filter that turns dataset into unstructured grid comprised of
//    a seed cell and a number of layers surrounding the seed cell as
//    specified by the user.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 09, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Oct 27 10:23:52 PDT 2000
//    Added argument to ExecuteDomain to match interited interface.
//
//    Jeremy Meredith, Thu Mar  1 13:29:27 PST 2001
//    Made attributes be stored as an OnionPeelAttributes class.
//
//    Jeremy Meredith, Sun Mar  4 16:59:57 PST 2001
//    Added a static Create method.
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001 
//    Changed ExecuteDomain to ExecuteData. 
//
//    Hank Childs, Wed Jun  6 08:58:51 PDT 2001
//    Renamed some methods to fit changes in base class.
//
//    Kathleen bonnell, Tue Oct 16 14:55:41 PDT 2001
//    Added VerifyInput, PerformRestriction. 
//
//    Hank Childs, Wed May 22 17:25:22 PDT 2002
//    Added BadSeedCell.
//
//    Kathleen Bonnell, Thu Aug 15 18:37:59 PDT 2002  
//    Added Pre/PostExecute methods. Added bool argument to BadSeedCell and
//    BadSeedCellCallback.  Added groupCategory, encounteredGhostSeed, and 
//    successfullyExecuted. 
//
// ****************************************************************************

class avtOnionPeelFilter : public avtPluginStreamer
{
  public:
                         avtOnionPeelFilter();
    virtual             ~avtOnionPeelFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtOnionPeelFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Selecting neighbors for onion peel"; };
    virtual void         ReleaseData(void);

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

    void                 BadSeed(int, int, bool);
    static void          BadSeedCallback(void *, int, int, bool);


  protected:
    OnionPeelAttributes   atts;
    vtkOnionPeelFilter   *opf;
    vtkPolyDataOnionPeelFilter   *poly_opf;

    int                   badSeed;
    int                   maximumIds;
    bool                  encounteredBadSeed;
    bool                  encounteredGhostSeed;
    bool                  groupCategory;
    bool                  successfullyExecuted;

    virtual vtkDataSet   *ExecuteData(vtkDataSet *, int, std::string);
    virtual void          PreExecute();
    virtual void          PostExecute();

    virtual void          VerifyInput(void);
    virtual void          RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);

};


#endif


