// ************************************************************************* //
//                          avtTwoPassDatasetQuery.h                         //
// ************************************************************************* //

#ifndef AVT_TWOPASS_DATASET_QUERY_H
#define AVT_TWOPASS_DATASET_QUERY_H

#include <query_exports.h>

#include <avtDatasetQuery.h>


// ****************************************************************************
//  Class:  avtTwoPassDatasetQuery
//
//  Purpose:
//    A specialization of avtDatasetQuery that executes the data tree in
//    two stages.  This can be useful if you need to perform one calculation
//    across all the data before you can perform the real calculation.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 15, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class avtTwoPassDatasetQuery : public avtDatasetQuery
{
  public:
                                avtTwoPassDatasetQuery();
    virtual                    ~avtTwoPassDatasetQuery();

  protected:
    virtual void                Execute1(vtkDataSet*, const int) = 0;
    virtual void                Execute2(vtkDataSet*, const int) = 0;
    virtual void                MidExecute(void) {;}

  private:
    virtual void                Execute(avtDataTree_p);
    virtual void                Execute(vtkDataSet*, const int) {}
    virtual void                ExecuteNthPass(avtDataTree_p, int);
};

#endif
