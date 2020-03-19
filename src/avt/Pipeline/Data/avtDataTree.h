// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtDataTree.h                               //
// ************************************************************************* //

#ifndef AVT_DATA_TREE_H
#define AVT_DATA_TREE_H

#include <pipeline_exports.h>

#include <vector>
#include <string>
#include <set>
#include <ref_ptr.h>

#include <avtDataRepresentation.h>

class vtkDataSet;
class avtDataTree;
class avtWebpage;

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
//    Hank Childs, Fri Jan  9 10:04:22 PST 2004
//    Added GetAllDomainIds.
//
//    Hank Childs, Thu Dec 21 10:06:47 PST 2006
//    Add support for debug dumps.
//
//    Hank Childs, Tue Dec 18 10:25:09 PST 2007
//    Change argument of assignment operator to be const, so that the compiler
//    won't insert a second, undesired assignment operator that does have
//    a const argument (and is used unexpectedly).
//
//    Kathleen Bonnell, Thu Feb 17 09:16:46 PST 2011
//    Added a PruneTree method that accepts a single string.
//
//    Cameron Christensen, Thursday, May 29, 2014
//    Added a couple of more constructors that take avtDataRepresentation.
//
//    Kathleen Biagas, Wed Jun  3 10:56:38 PDT 2015
//    Added methods for constructing leaves, to aid in keeping the tree depth
//    as shallow as possible.
//
//    Burlen Loring, Sun Sep  6 14:58:03 PDT 2015
//    Changed the return type of GetNumberOfCells to long long
//
// ****************************************************************************

class PIPELINE_API avtDataTree
{
  public:
                             avtDataTree();
                             avtDataTree(vtkDataSet *, int);
                             avtDataTree(vtkDataSet *, int, std::string s);
                             avtDataTree(avtDataRepresentation &);
                             avtDataTree(avtDataRepresentation *);
                             avtDataTree(int, vtkDataSet **, int *);
                             avtDataTree(int, vtkDataSet **,std::vector<int>&);
                             avtDataTree(int, vtkDataSet **,std::vector<int>&,
                                         std::vector<std::string>&);
                             avtDataTree(int, vtkDataSet **, int);
                             avtDataTree(int, vtkDataSet **, int,
                                         std::vector<std::string>&);
                             avtDataTree(int, vtkDataSet **,int,std::string &);
                             avtDataTree(int, avtDataRepresentation *);
                             avtDataTree(int, avtDataRepresentation **);
                             avtDataTree(avtDataTree_p, bool dontCopyData = false );
                             avtDataTree(int, avtDataTree_p *);

    virtual                 ~avtDataTree();

    avtDataTree             &operator=(const avtDataTree&);
    avtDataTree             &operator=(const avtDataTree*);

    avtDataTree_p            GetChild(int);
    avtDataRepresentation   &GetDataRepresentation(void); 
    long long                GetNumberOfCells(int topoDim, bool polysOnly) const;

    vtkDataSet              *GetSingleLeaf(void);
    std::string              GetDatasetAsString();
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
    void                     GetAllDomainIds(std::vector<int> &);
    void                     GetAllLabels(std::vector<std::string> &);
    void                     GetAllUniqueLabels(std::vector<std::string> &);
    avtDataTree_p            PruneTree(const std::vector<int> &);
    avtDataTree_p            PruneTree(const std::vector<int> &, std::vector<int> &);
    avtDataTree_p            PruneTree(const std::vector<std::string> &);
    avtDataTree_p            PruneTree(const std::string &);
    avtDataTree_p            PruneTree(const std::vector<std::string> &, std::vector<std::string>&);

    void                     WriteTreeStructure(ostream &, int indent = 0);

    void                     DebugDump(avtWebpage *, const char *, int, int);

  private:
    int                      nChildren;
    avtDataTree_p           *children;
    avtDataRepresentation   *dataRep; 

    void                     ConstructLeaf(vtkDataSet *, int);
    void                     ConstructLeaf(vtkDataSet *, int, std::string);
    void                     AddLeafToList(vtkDataSet**, int &); 
    void                     GetUniqueLabels(std::vector<std::string> &, std::set<std::string> &);
};

#endif
