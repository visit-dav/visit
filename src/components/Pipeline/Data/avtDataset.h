// ************************************************************************* //
//                                avtDataset.h                               //
// ************************************************************************* //

#ifndef AVT_DATASET_H
#define AVT_DATASET_H
#include <pipeline_exports.h>

#include <avtCommonDataFunctions.h>
#include <avtDataObject.h>
#include <avtDataTree.h>

class   avtDataRepresentation;


// ****************************************************************************
//  Class: avtDataset
//
//  Purpose:
//      The only example of a data object in our component base.  It contains
//      multiple blocks of a dataset, with each block in the form of a 
//      vtkDataSet.
//
//  Programmer: Hank Childs
//  Creation:   June 28, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  9 17:11:18 PST 2001 
//    Changed representation of domains to avtDomainTree. 
//
//    Kathleen Bonnell, Thu Apr  5 13:34:11 PDT 2001 
//    avtDomainTree renamed as avtDataTree, avtDomain renamed 
//    as avtDataRepresentation.  What used to be domains, an array of 
//    avtDomainTrees, is now just one avtDataTree. 
//
//    Hank Childs, Tue May 22 20:56:19 PDT 2001
//    Use database specifications instead of domain lists.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 200 
//    Added method to write out tree structure. 
//
//    Hank Childs, Fri Sep 28 13:41:36 PDT 2001
//    Add Compact method.
//
//    Hank Childs, Fri Mar 15 17:14:25 PST 2002
//    Pushed some examination routines into the new object avtDatasetExaminer.
//    Removed comments related to that class.
//
// ****************************************************************************

class PIPELINE_API avtDataset : public avtDataObject
{
    friend                   class avtDatasetSink;
    friend                   class avtDatasetSource;
    friend                   class avtSourceFromAVTDataset;
    friend                   class avtDatasetExaminer;

  public:
                             avtDataset(avtDataObjectSource *);
                             avtDataset(ref_ptr<avtDataset> ds, bool dontCopyData = false);
                             avtDataset(vtkDataSet *);
    virtual                 ~avtDataset() {;};

    virtual const char      *GetType(void)  { return "avtDataset"; };
    virtual int              GetNumberOfCells(bool polysOnly = false) const;
    virtual void             ReleaseData(void);

    virtual avtDataObject   *Instance(void);
    virtual avtDataObjectWriter 
                            *InstantiateWriter(void);

    void                     SetActiveVariable(const char *);

    void                     WriteTreeStructure(ostream &os, int indent = 0);
    void                     Compact(void);

  protected:
    avtDataTree_p            dataTree;

    void                     SetDataTree(avtDataTree_p);
    void                     SetDataTree(avtDataRepresentation &);
    avtDataTree_p           &GetDataTree() { return dataTree; };

    virtual void             DerivedCopy(avtDataObject *);
    virtual void             DerivedMerge(avtDataObject *);
};


typedef ref_ptr<avtDataset> avtDataset_p;


#endif


