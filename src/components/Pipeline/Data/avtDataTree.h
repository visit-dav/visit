// ************************************************************************* //
//                               avtDataTree.h                               //
// ************************************************************************* //

#ifndef AVT_DATA_TREE_H
#define AVT_DATA_TREE_H
#include <pipeline_exports.h>

#include <vector>
#include <set>
#include <ref_ptr.h>

#include <avtDataRepresentation.h>

using std::vector;
using std::string;
using std::set;
 
class vtkDataSet;
class avtDataTree;

typedef ref_ptr<avtDataTree> avtDataTree_p;

typedef void (*TraverseFunc)(avtDataRepresentation &, void *, bool &);


// ****************************************************************************
//  Class: avtDataTree
//
//  Purpose:
//     A tree structure allowing for a hierarchy of avtDataRepresentations.  
//     A leaf in the tree is a vtkDataset (wrapped as an avtDataRepresentation).
//     Internal nodes are themselves avtDataTrees.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 1, 2001 
//
//  Modifications:
//
//    Hank Childs, Wed Mar  7 15:43:34 PST 2001
//    Removed methods GetDomain(int), Notify, and SetParent as well as
//    data member parent.
//
//    Hank Childs, Sat Apr  7 14:15:02 PDT 2001
//    Added method HasData to determine if there is data.
//
//    Kathleen Bonnell, Thu Apr  5 11:52:25 PDT 2001
//    Renamed as avtDataTree.  avtDomain renamed avtDataRepresentation.
//    Added Merge method.  Added Traverse method. Removed CreateDomain method.
//
//    Hank Childs, Tue Jun  5 09:55:39 PDT 2001
//    Added method GetChunk.
//
//    Hank Childs, Tue Jul 17 09:58:37 PDT 2001
//    Allow for an empty data tree.
//
//    Kathleen Bonnell, Mon Aug 13 13:05:59 PDT 2001 
//    Add method to write tree structure to desired output stream.
//
//    Kathleen Bonnell, Wed Sep 19 12:41:15 PDT 2001 
//    Removed GetChunk method (superceded by PruneTree).  
//
//    Kathleen Bonnell, Mon Sep 24 15:24:18 PDT 2001 
//    Added GetAllLabels method. 
//
//    Kathleen Bonnell, Thu Nov  1 11:10:39 PST 2001 
//    Added IsEmpty method. 
//
//    Hank Childs, Mon Nov 12 14:27:03 PST 2001
//    Added GetSingleLeaf.
//
//    Kathleen Bonnell, Mon Apr 29 17:49:25 PDT 2002 
//    Added GetAllUniqueLabels and GetUniqueLabels.
//
// ****************************************************************************

class PIPELINE_API avtDataTree
{
  public:
                             avtDataTree();
                             avtDataTree(vtkDataSet *, int);
                             avtDataTree(vtkDataSet *, int, string s);
                             avtDataTree(avtDataRepresentation &);
                             avtDataTree(int, vtkDataSet **, int *);
                             avtDataTree(int, vtkDataSet **,std::vector<int>&);
                             avtDataTree(int, vtkDataSet **, int);
                             avtDataTree(int, vtkDataSet **, int,
                                         std::vector<std::string>&);
                             avtDataTree(int, vtkDataSet **,int,std::string &);
                             avtDataTree(int, avtDataRepresentation *);
                             avtDataTree(avtDataTree_p, bool dontCopyData = false );
                             avtDataTree(int, avtDataTree_p *);
   
    virtual                 ~avtDataTree();

    avtDataTree             &operator =(avtDataTree&);

    avtDataTree_p            GetChild(int);
    avtDataRepresentation   &GetDataRepresentation(void); 

    vtkDataSet              *GetSingleLeaf(void);
    int                      GetNumberOfLeaves(void);
    void                     Merge(avtDataTree_p);

    int                      GetNChildren(void) {return nChildren; } ;
    bool                     ChildIsPresent(int i) 
                                 { return *(children[i]) != NULL; } ;
    bool                     HasData(void) 
                                 { return (dataRep == NULL ? false : true); };
    bool                     IsEmpty(void);

    void                     Traverse(TraverseFunc, void *, bool &);

    vtkDataSet             **GetAllLeaves(int &);
    void                     GetAllLabels(vector<string> &);
    void                     GetAllUniqueLabels(vector<string> &);
    avtDataTree_p            PruneTree(const vector<int> &);
    avtDataTree_p            PruneTree(const vector<int> &, vector<int> &);
    avtDataTree_p            PruneTree(const vector<string> &);
    avtDataTree_p            PruneTree(const vector<string> &, vector<string>&);

    void                     WriteTreeStructure(ostream &, int indent = 0);


  private:
    int                      nChildren;
    avtDataTree_p           *children;
    avtDataRepresentation   *dataRep; 

    void                     AddLeafToList(vtkDataSet**, int &); 
    void                     GetUniqueLabels(vector<string> &, set<string> &);
};


#endif
