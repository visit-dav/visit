// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtDatasetVerifier.h                           //
// ************************************************************************* //

#ifndef AVT_DATASET_VERIFIER_H
#define AVT_DATASET_VERIFIER_H

#include <pipeline_exports.h>

#include <avtDataTree.h>

class     vtkCellArray;
class     vtkDataArray;
class     vtkDataSet;
class     vtkDataSetAttributes;


// ****************************************************************************
//  Class: avtDatasetVerifier
//
//  Purpose:
//      Looks through an avtDataTree and verifies that each dataset looks
//      reasonable.  Issues warnings and corrects them if they are not.
//
//  Programmer: Hank Childs
//  Creation:   October 18, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Fri Jan  9 09:36:01 PST 2004
//    Add a routine that is accessible for the generic database.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Kathleen Bonnell, Fri Nov 12 08:22:29 PST 2004 
//    Changed arguments for CorrectVarMismatch, to make the method able to
//    handle more var types. 
//
//    Hank Childs, Tue Jul  5 16:22:56 PDT 2005
//    Add variable name to IssueWarning call.
//
//    Hank Childs, Sat Jan  1 12:42:28 PST 2011
//    Add methods for -safe mode.
//
// ****************************************************************************

class PIPELINE_API avtDatasetVerifier
{
  public:
                avtDatasetVerifier();
    virtual    ~avtDatasetVerifier();

    void        VerifyDatasets(int, vtkDataSet **, std::vector<int> &domains);

  protected:
    bool        issuedWarningForVarMismatch;
    bool        issuedSafeModeWarning;

    void        VerifyDataset(vtkDataSet *, int);
    void        CheckArray(int, vtkDataArray *, const char *);
    void        CheckConnectivity(int, int, vtkCellArray *, const char *);
    void        CorrectVarMismatch(vtkDataArray *, vtkDataSetAttributes*, int); 

    void        IssueVarMismatchWarning(int, int, bool, int, const char *);
};


#endif


