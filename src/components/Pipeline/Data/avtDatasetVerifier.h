// ************************************************************************* //
//                            avtDatasetVerifier.h                           //
// ************************************************************************* //

#ifndef AVT_DATASET_VERIFIER_H
#define AVT_DATASET_VERIFIER_H

#include <pipeline_exports.h>

#include <avtDataTree.h>

class     vtkDataSet;
class     vtkDataArray;


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
// ****************************************************************************

class PIPELINE_API avtDatasetVerifier
{
  public:
                avtDatasetVerifier();
    virtual    ~avtDatasetVerifier();

    void        VerifyDatasets(int, vtkDataSet **, std::vector<int> &domains);

  protected:
    bool        issuedWarningForVarMismatch;

    void        VerifyDataset(vtkDataSet *, int);
    void        CorrectVarMismatch(vtkDataSet *, vtkDataArray *, bool);

    void        IssueVarMismatchWarning(int, int, bool, int);
};


#endif


