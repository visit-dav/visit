// ************************************************************************* //
//                                avtStreamer.h                              //
// ************************************************************************* //

#ifndef AVT_STREAMER_H
#define AVT_STREAMER_H
#include <pipeline_exports.h>


#include <avtDataTreeStreamer.h>


// ****************************************************************************
//  Class: avtStreamer
//
//  Purpose:
//      A derived type of avtDatasetToDatasetFilter.  This will "stream" 
//      datasets through a single filter one at a time during execution.  It
//      could be part of the actual avtDatasetToDatasetFilter class, since
//      almost every derived type of avtDatasetToDatasetFilter will be 
//      interested in streaming their domains through, but it was made a
//      separate class to separate what functionality was for a filter (or
//      process object) in the pipeline and what sent domains through vtk
//      filters one at a time.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Thu Sep 28 13:05:02 PDT 2000
//    Made ExecuteDomain take one vtkDataSet as input and and return
//    a new output one.
//
//    Kathleen Bonnell, Feb  9 14:47:10 PST 2001  
//    Removed 'Execute' method, and made this class inherit from 
//    avtDomainTreeStreamer.  Allows for derived types to still
//    return vtkDataSet * from ExecuteDomain, but now wrapped in
//    avtDomainTree.
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001 
//    Change inheritance to avtDataTreeStreamer. 
//
//    Kathleen Bonnell, Wed Sep 19 13:35:35 PDT 200 
//    Added string argument to Execute method. 
//
//    Hank Childs, Fri Feb  1 14:48:15 PST 2002
//    Added mechanism for managing memory for derived types.
//
//    Hank Childs, Tue Sep 10 13:13:01 PDT 2002
//    Better support for releasing data.
//
// **************************************************************************** 

class PIPELINE_API avtStreamer : public avtDataTreeStreamer
{
  public:
                             avtStreamer();
    virtual                 ~avtStreamer();

    virtual void             ReleaseData(void);
    static void              DebugDump(bool d) {debugDump = d;}

  protected:
    vtkDataSet              *lastDataset;

    virtual avtDataTree_p    ExecuteDataTree(vtkDataSet *, int, std::string);
    virtual vtkDataSet      *ExecuteData(vtkDataSet *, int, std::string) = 0;

    void                     ManageMemory(vtkDataSet *);
    static bool              debugDump;
};


#endif


