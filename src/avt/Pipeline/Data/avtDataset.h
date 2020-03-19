// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
class   avtIntervalTree;
class   avtNamedSelectionExtension;
class   avtWebpage;

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
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Mon Aug 29 14:57:42 PDT 2005
//    Add friend status for avtCMFEExpression.
//
//    Hank Childs, Thu Sep 22 17:34:12 PDT 2005
//    Add friend status for avtCompactTreeFilter.
//
//    Hank Childs, Sat Jan 21 13:17:12 PST 2006
//    Add friend status for avtSymmEvalExpression.
//
//    Hank Childs, Wed Aug  2 15:10:42 PDT 2006
//    Add friend status for avtLineScanQuery.
//
//    Hank Childs, Thu Dec 21 09:58:57 PST 2006
//    Added method for debug dumps.
//
//    Hank Childs, Fri May 18 16:51:07 PDT 2007
//    Add a constructor for creating a new data set with a specific data tree.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Hank Childs, Thu Jan 24 12:58:59 PST 2008
//    Add friend status for avtLineSurfaceFilter.
//
//    Hank Childs, Thu Jan 24 15:48:41 PST 2008
//    Add friend status for avtExecuteThenTimeLoopFilter.
//
//    Hank Childs, Mon Jun 16 12:25:31 PDT 2008
//    Add method CalculateSpatialIntervalTree.
//
//    Hank Childs, Tue Feb  3 11:26:05 PST 2009
//    Add friend status for avtNamedSelectionManager.
//
//    Hank Childs, Sun Feb 15 10:12:22 PST 2009
//    Add friend status for avtTimeIteratorExpression.
//
//    Eric Brugger, Wed Jun 30 13:43:30 PDT 2010
//    Add friend status for avtXRayImageQuery.
//
//    Dave Pugmire, Fri Jul  2 14:22:34 EDT 2010
//    Add friend status for avtResampleExpression.
//
//    Hank Childs, Thu Sep  2 07:54:12 PDT 2010
//    Add method RenumberDomainIDs.
//
//    Hank Childs, Fri Sep 10 19:23:26 PDT 2010
//    Add options to RenumberDomainIDs and CalculateSpatialIntervalTree
//    to limit the calculations to the current processor only.
//
//    Dave Pugmire, Mon Mar 26 13:50:09 EDT 2012
//    Add avtExtremeValueAnalysisFilter
//
//    Burlen Loring, Sun Sep  6 14:58:03 PDT 2015
//    Changed the return type of GetNumberOfCells to long long
//
// ****************************************************************************

class PIPELINE_API avtDataset : public avtDataObject
{
    friend                   class avtDatasetSink;
    friend                   class avtDatasetSource;
    friend                   class avtSourceFromAVTDataset;
    friend                   class avtDatasetExaminer;
    friend                   class avtCMFEExpression;
    friend                   class avtCompactTreeFilter;
    friend                   class avtSymmEvalExpression;
    friend                   class avtLineScanQuery;
    friend                   class avtLineSurfaceFilter;
    friend                   class avtExecuteThenTimeLoopFilter;
    friend                   class avtNamedSelectionExtension;
    friend                   class avtTimeIteratorExpression;
    friend                   class avtXRayImageQuery;
    friend                   class avtResampleExpression;
    friend                   class avtIndexSelectFilter;

  public:
                             avtDataset(avtDataObjectSource *);
                             avtDataset(ref_ptr<avtDataset> ds,
                                        bool dontCopyData = false);
                             avtDataset(ref_ptr<avtDataset> ds,
                                        avtDataTree_p);
                             avtDataset(vtkDataSet *);
                             avtDataset(vtkDataSet *, avtDataset *);
    virtual                 ~avtDataset();

    virtual const char      *GetType(void)  { return "avtDataset"; };
    virtual long long        GetNumberOfCells(bool polysOnly = false) const;
    virtual void             ReleaseData(void);

    virtual avtDataObject   *Instance(void);
    virtual avtDataObjectWriter 
                            *InstantiateWriter(void);

    void                     SetActiveVariable(const char *);

    void                     WriteTreeStructure(ostream &os, int indent = 0);
    void                     Compact(void);

    avtIntervalTree         *CalculateSpatialIntervalTree(bool acrossAllProcs = true);
    void                     RenumberDomainIDs(bool acrossAllProcs = true);
    virtual void             DebugDump(avtWebpage *, const char *);

    std::string              GetDatasetAsString();
  protected:
    avtDataTree_p            dataTree;

    void                     SetDataTree(avtDataTree_p);
    void                     SetDataTree(avtDataRepresentation &);
    avtDataTree_p           &GetDataTree() { return dataTree; };

    virtual void             DerivedCopy(avtDataObject *);
    virtual void             DerivedMerge(avtDataObject *);

  private:
    // This method is defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define it to do something
    // meaningful, that's fine.
    avtDataset              &operator=(const avtDataset &) { return *this; };
};


typedef ref_ptr<avtDataset> avtDataset_p;


#endif


